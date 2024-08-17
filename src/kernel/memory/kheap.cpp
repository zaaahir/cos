#include "memory/kheap.h"
#include "x86_64.h"

namespace Memory {
    /*
        For the kernel heap manager, we use a binary search tree organising free sections in the current heap and ordering them by size.
        This means that sufficient size spaces can be quickly found and allocated.
        Each allocation or free space in memory also has surrounding tags to quickly point to the heap and allow for a fast kmalloc/kfree.
    */

    bool entry_less_than_predicate(HeapIndexEntry a, HeapIndexEntry b)
    {
        return a.size < b.size;
    }

    KernelHeapManager::KernelHeapManager() : m_bst(Common::BinarySearchTree<HeapIndexEntry>(reinterpret_cast<void*>(HEAP_BASE), HEAP_INDEX_SIZE, &entry_less_than_predicate))
    {
        m_nextIndexPagePointer = reinterpret_cast<uint8_t*>(HEAP_BASE);
        m_nextHeapPagePointer = reinterpret_cast<uint8_t*>(HEAP_BASE + HEAP_INDEX_SIZE);

        // Allocate first pages for heap and index (binary search tree).
        Memory::MemoryManager::instance().alloc_page(VirtualMemoryAllocationRequest(VirtualAddress(m_nextIndexPagePointer), true, true));
        Memory::MemoryManager::instance().alloc_page(VirtualMemoryAllocationRequest(VirtualAddress(m_nextHeapPagePointer), true, true));

        m_nextIndexPagePointer += PAGE_4KiB;
        m_nextHeapPagePointer += PAGE_4KiB;

        // Create the first entry which is the size of the whole page.
        auto entry = HeapIndexEntry();
        entry.ptr = m_nextHeapPagePointer - PAGE_4KiB + sizeof(HeapChunkHeader);
        entry.size = Memory::PAGE_4KiB - sizeof(HeapChunkHeader) - sizeof(HeapChunkFooter);
        insert_entry(entry);
        write_entry_tags(entry, true);
    }

    // Write header and footer tags for memory chunks in memory.
    void KernelHeapManager::write_entry_tags(HeapIndexEntry entry, bool isHole)
    {
        uint8_t* start = entry.ptr - sizeof(HeapChunkHeader);
        auto heapChunkHeader = new (start) HeapChunkHeader();
        heapChunkHeader->isHole = isHole;
        heapChunkHeader->size = entry.size;
        auto heapChunkFooter = new (entry.ptr + entry.size) HeapChunkFooter();
        heapChunkFooter->size = entry.size;
    }

    void KernelHeapManager::insert_entry(HeapIndexEntry entry)
    {
        // If the binary search tree page range is too small, expand it.
        if (m_bst.get_size() + m_bst.get_size_of_one_node() > reinterpret_cast<uint64_t>(m_nextIndexPagePointer - HEAP_BASE))
        {
            Memory::MemoryManager::instance().alloc_page(Memory::VirtualMemoryAllocationRequest(m_nextIndexPagePointer, true, true));
            m_nextIndexPagePointer += PAGE_4KiB;
        }
        m_bst.insert_element(entry);
    }

    void KernelHeapManager::remove_entry(HeapIndexEntry entry)
    {
        m_bst.remove_element(entry);
        // If the last page of the binary search tree page range is unused, free it.
        if (m_bst.get_size() < reinterpret_cast<uint64_t>(m_nextIndexPagePointer - HEAP_BASE - Memory::PAGE_4KiB))
        {
            Memory::MemoryManager::instance().free_page(VirtualAddress((m_nextIndexPagePointer) - Memory::PAGE_4KiB));
            m_nextIndexPagePointer -= PAGE_4KiB;
        }
    }

    HeapIndexEntry KernelHeapManager::get_entry(HeapChunkHeader* header)
    {
        HeapIndexEntry entry = HeapIndexEntry();
        entry.ptr = reinterpret_cast<uint8_t*>(header) + sizeof(HeapChunkHeader);
        entry.size = header->size;
        return entry;
    }

    HeapIndexEntry KernelHeapManager::get_entry(HeapChunkFooter* footer)
    {
        HeapIndexEntry entry = HeapIndexEntry();
        entry.ptr = reinterpret_cast<uint8_t*>(footer) - footer->size;
        entry.size = footer->size;
        return entry;
    }

    HeapChunkHeader* KernelHeapManager::get_corresponding_header(HeapChunkFooter* footer)
    {
        return reinterpret_cast<HeapChunkHeader*>(reinterpret_cast<uint8_t*>(footer) - sizeof(HeapChunkHeader) - footer->size);
    }

    HeapChunkHeader* KernelHeapManager::get_next_header(HeapChunkHeader* header)
    {
        auto headerPointer = reinterpret_cast<uint8_t*>(header) + sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter) + header->size;
        if (headerPointer >= m_nextHeapPagePointer)
            return nullptr;
        return reinterpret_cast<HeapChunkHeader*>(headerPointer);
    }

    HeapChunkHeader* KernelHeapManager::get_prev_header(HeapChunkHeader* header)
    {
        auto footerPointer = reinterpret_cast<HeapChunkFooter*>(header) - 1;
        if (reinterpret_cast<uint64_t>(footerPointer) < static_cast<uint64_t>(HEAP_BASE + HEAP_INDEX_SIZE))
            return nullptr;
        return get_corresponding_header(footerPointer);
    }

    // Expand the heap by adding an extra page at the end and merging the previous chunk if it was a hole.
    void KernelHeapManager::expand()
    {
        Memory::MemoryManager::instance().alloc_page(VirtualMemoryAllocationRequest(m_nextHeapPagePointer, true, true));
        auto oldFooter = reinterpret_cast<HeapChunkFooter*>(m_nextHeapPagePointer) - 1;
        auto oldHeader = get_corresponding_header(oldFooter);

        if (oldHeader->isHole)
        {
            // We can simply make the previous entry 4KiB larger.
            HeapIndexEntry oldEntry = get_entry(oldFooter);
            remove_entry(oldEntry);
            oldEntry.size += PAGE_4KiB;
            insert_entry(oldEntry);
            write_entry_tags(oldEntry, true);
        } else
        {
            // We need to insert a new entry for the new page.
            HeapIndexEntry newEntry = HeapIndexEntry();
            newEntry.ptr = m_nextHeapPagePointer + sizeof(HeapChunkHeader);
            newEntry.size = PAGE_4KiB - sizeof(HeapChunkHeader) - sizeof(HeapChunkFooter);
            insert_entry(newEntry);
            write_entry_tags(newEntry, true);
        }

        m_nextHeapPagePointer += PAGE_4KiB;
    }

    void* KernelHeapManager::kmalloc(uint64_t size, uint64_t flags)
    {
        m_spinlock.acquire();

        // If we need an aligned allocation, we need to request a bigger allocation and provide a pointer into a word-aligned address in the allocation.
        uint64_t reqSize = size + (flags & WORD_ALIGN ? sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter) + sizeof(uint64_t) : 0);

        auto entry = find_smallest_entry_greater_than(reqSize);

        // Keep expanding the heap until we find a sufficiently sized entry.
        while (!entry)
        {
            expand();
            entry = find_smallest_entry_greater_than(reqSize);
        }

        HeapIndexEntry backEntry = *entry;

        remove_entry(*entry);
        if (flags & WORD_ALIGN)
        {
            // We need to split into two entries.
            HeapIndexEntry frontEntry = backEntry;
            frontEntry.size = 8 - ((2 * sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter)) % 8);
            insert_entry(frontEntry);
            write_entry_tags(frontEntry, true);
            backEntry.ptr += frontEntry.size + sizeof(HeapChunkFooter) + sizeof(HeapChunkHeader);
            backEntry.size -= frontEntry.size + sizeof(HeapChunkFooter) + sizeof(HeapChunkHeader);
        }
        // The entry can be split into two - the allocation and the leftover free hole.
        if (backEntry.size >= size + sizeof(HeapChunkFooter) + sizeof(HeapChunkHeader))
        {
            uint64_t oldEntrySize = backEntry.size;
            backEntry.size = size;
            HeapIndexEntry holeEntry = backEntry;
            holeEntry.ptr += backEntry.size + sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter);
            holeEntry.size = oldEntrySize - size - sizeof(HeapChunkHeader) - sizeof(HeapChunkFooter);
            write_entry_tags(holeEntry, true);
            insert_entry(holeEntry);
        }
        write_entry_tags(backEntry, false);
        m_spinlock.release();
        return backEntry.ptr;
    }

    HeapChunkHeader* KernelHeapManager::get_header(HeapIndexEntry entry)
    {
        return reinterpret_cast<HeapChunkHeader*>(entry.ptr) - 1;
    }

    // If the next entry is also a hole, combine the two into a larger entry.
    void KernelHeapManager::merge_forwards(HeapIndexEntry* entry)
    {
        auto nextHeader = get_next_header(get_header(*entry));
        if (nextHeader)
        {
            if (nextHeader->isHole)
            {
                auto nextEntry = get_entry(nextHeader);
                // Remove the two smaller entries.
                remove_entry(*entry);
                remove_entry(nextEntry);
                entry->size += nextHeader->size + sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter);
                // Insert the larger, combined entry.
                insert_entry(*entry);
                write_entry_tags(*entry, true);
            }
        }
    }

    // If the previous entry is also a hole, combine the two into a larger entry.
    void KernelHeapManager::merge_backwards(HeapIndexEntry* entry)
    {
        auto prevHeader = get_prev_header(get_header(*entry));
        if (prevHeader)
        {
            if (prevHeader->isHole)
            {
                auto prevEntry = get_entry(prevHeader);
                // Remove the two smaller entries.
                remove_entry(*entry);
                remove_entry(prevEntry);
                entry->size += prevHeader->size + sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter);
                entry->ptr -= prevHeader->size + sizeof(HeapChunkHeader) + sizeof(HeapChunkFooter);
                // Insert the larger, combined entry.
                insert_entry(*entry);
                write_entry_tags(*entry, true);
            }
        }
    }

    void KernelHeapManager::kfree(void* ptr)
    {
        m_spinlock.acquire();

        auto header = reinterpret_cast<HeapChunkHeader*>(ptr) - 1;
        // Add the entry into the index binary search tree.
        auto entry = get_entry(header);
        insert_entry(entry);
        write_entry_tags(entry, true);

        // Try and merge with holes either side, if they exist.
        merge_forwards(&entry);
        merge_backwards(&entry);

        m_spinlock.release();
    }

    HeapIndexEntry* KernelHeapManager::find_smallest_entry_greater_than(uint64_t size)
    {
        // We insert a searchEntry that is of the requested size and find the successor to the node in the binary search tree.
        // The successor is the smallest entry that is greater than the searchEntry.
        if (m_bst.get_size() + m_bst.get_size_of_one_node() > reinterpret_cast<uint64_t>(m_nextIndexPagePointer - HEAP_BASE))
        {
            Memory::MemoryManager::instance().alloc_page(VirtualMemoryAllocationRequest(m_nextIndexPagePointer, true, true));
            m_nextIndexPagePointer += PAGE_4KiB;
        }
        HeapIndexEntry searchEntry = HeapIndexEntry();
        searchEntry.size = size;
        auto retEntry = m_bst.get_successor(searchEntry);
        if (m_bst.get_size() < reinterpret_cast<uint64_t>(m_nextIndexPagePointer - HEAP_BASE - PAGE_4KiB))
        {
            Memory::MemoryManager::instance().free_page(VirtualMemoryFreeRequest(m_nextIndexPagePointer - PAGE_4KiB));
            m_nextIndexPagePointer -= PAGE_4KiB;
        }
        return retEntry;
    }
}
