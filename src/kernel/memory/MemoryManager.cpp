#include "memory/MemoryManager.h"
#include "panic.h"
#include "memory/address.h"

namespace Memory {
    
    void MemoryManager::init_physical_region(PhysicalAddress base, uint64_t size)
    {
        auto align = reinterpret_cast<uint64_t>(base.get()) / PHYSICAL_BLOCK_SIZE;
        auto blocks = size / PHYSICAL_BLOCK_SIZE;
        for (auto i = 0; i < blocks; i++)
        {
            m_physicalBitmap.unset(align + i);
        }
        m_physicalBitmap.set(0);
    }

    void MemoryManager::deinit_physical_region(PhysicalAddress base, uint64_t size)
    {
        auto align = reinterpret_cast<uint64_t>(base.get()) / PHYSICAL_BLOCK_SIZE;
        auto blocks = size / PHYSICAL_BLOCK_SIZE;

        // We want to ensure that absolutely all of the requested area is deinitialised - we do not mind deinitialising more.
        if (size % PHYSICAL_BLOCK_SIZE)
        {
            blocks++;
        }

        if ((size % PHYSICAL_BLOCK_SIZE) + (reinterpret_cast<uint64_t>(base.get()) % PHYSICAL_BLOCK_SIZE) > PHYSICAL_BLOCK_SIZE)
        {
            blocks++;
        }

        for (auto i = 0; i < blocks; i++)
        {
            m_physicalBitmap.set(align + i);
        }
    }

    PhysicalAddress MemoryManager::alloc_physical_block()
    {
        auto frame = m_physicalBitmap.first_unset();
        auto physAddress = frame * PHYSICAL_BLOCK_SIZE;
        
        if (frame == -1)
        {
            Kernel::panic("Physical memory manager unable to allocate memory, out of memory?");
        }

        m_physicalBitmap.set(frame);

        return PhysicalAddress(physAddress);
    }

    void MemoryManager::free_physical_block(PhysicalAddress block)
    {
        auto physAddress = reinterpret_cast<uint64_t>(block.get());
        auto frame = physAddress / PHYSICAL_BLOCK_SIZE;
        m_physicalBitmap.unset(frame);
    }

    void MemoryManager::protect_physical_regions(PhysicalAddress kernelEnd)
    {
        m_physicalMemorySize = 0;
        auto it = Multiboot::MultibootManager::instance().first(MULTIBOOT_TAG_TYPE_MMAP);

        if (it.is_end())
        {
            Kernel::panic("The physical memory manager was unable to find a memory map tag.");
        }
        
        // Find the end of physical memory. We treat this as the physical memory size for creating the bitmap.
        auto tag = reinterpret_cast<multiboot_tag_mmap*>(&(*it));
        for (auto entry = tag->entries;
            reinterpret_cast<uint64_t>(entry) < reinterpret_cast<uint64_t>(tag) + tag->size;
            entry = reinterpret_cast<multiboot_mmap_entry*>((reinterpret_cast<uint64_t>(entry) + tag->entry_size)))
        {
            if (entry->type == 1)
            {
                auto entryEnd = entry->addr + entry->len;
                if (entryEnd > m_physicalMemorySize) { m_physicalMemorySize = entryEnd; }
            }
        }

        // Calculate size of physical memory bitmap.
        auto numBits = m_physicalMemorySize / PHYSICAL_BLOCK_SIZE;
        if (m_physicalMemorySize % PHYSICAL_BLOCK_SIZE) { numBits++; }

        auto numBytes = numBits / PhysicalBitmap::CHAR_BIT;
        if (numBytes % PhysicalBitmap::CHAR_BIT) { numBytes++; }
        
        // We place the bitmap at the end of physical memory.
        m_physicalBitmap.initialise(VirtualAddress(PhysicalAddress(m_physicalMemorySize - numBytes)).get(), numBits);

        auto memoryBitmapPageStart = BYTE_ALIGN_UP(m_physicalMemorySize - numBytes, PHYSICAL_BLOCK_SIZE);

        for (auto entry = tag->entries;
            reinterpret_cast<uint64_t>(entry) < reinterpret_cast<uint64_t>(tag) + tag->size;
            entry = reinterpret_cast<multiboot_mmap_entry*>((reinterpret_cast<uint64_t>(entry) + tag->entry_size)))
        {
            if (entry->type == 1)
            {
                auto alignedAddress = BYTE_ALIGN_UP(entry->addr, PHYSICAL_BLOCK_SIZE);
                init_physical_region(alignedAddress, entry->len);
            }
        }
        deinit_physical_region(memoryBitmapPageStart, m_physicalMemorySize - memoryBitmapPageStart);
        deinit_physical_region(PhysicalAddress(static_cast<uint64_t>(0)), reinterpret_cast<uint64_t>(kernelEnd.get()));
        auto multibootStructure = Multiboot::MultibootManager::instance().get_structure();
        deinit_physical_region(reinterpret_cast<uint64_t>(multibootStructure), multibootStructure->total_size);
    }

    void GenericEntry::set_access_flags(VirtualMemoryMapRequest request)
    {
        set_flag(Flag::PRESENT);
        if (request.allowUserAccess)
            set_flag(Flag::USER_ACCESS);
        
        if (request.allowWrite)
            set_flag(Flag::WRITEABLE);
        
    }

    void GenericEntry::prepare_table_for_entry(VirtualMemoryMapRequest request)
    {
        if (!get_flag(Flag::PRESENT) || get_flag(Flag::PAGE_SIZE))
        {
            PhysicalAddress newTable = MemoryManager::instance().alloc_physical_block();
            memset(VirtualAddress(newTable).get(), 0, sizeof(GenericTable));
            set_frame(newTable);
            clear_flag(Flag::PAGE_SIZE);
        }
        set_access_flags(request);
    }

    void GenericEntry::prepare_page_for_entry(VirtualMemoryMapRequest request)
    {
        if (request.pageSize != PAGE_4KiB)
            set_flag(Flag::PAGE_SIZE);
        set_frame(request.physicalAddress);
        set_access_flags(request);
    }

    void GenericEntry::free_table()
    {
        // If the PAGE_SIZE flag is set, then this is not a table but rather a page entry.
        if (get_flag(Flag::PRESENT) && !get_flag(Flag::PAGE_SIZE))
        {
            MemoryManager::instance().free_physical_block(get_frame());
            m_entry = 0;
        }
    }
