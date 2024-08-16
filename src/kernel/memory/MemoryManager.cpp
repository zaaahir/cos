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
