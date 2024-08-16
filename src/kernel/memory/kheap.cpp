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
