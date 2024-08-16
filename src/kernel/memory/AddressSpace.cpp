#include "memory/AddressSpace.h"
#include "memory/MemoryManager.h"

namespace Memory
{
    MappedRegion RegionableVirtualAddressSpace::get_available_region(uint64_t size)
    {
        uint64_t regionBegin = PAGE_4KiB;
        for (auto it = regions.first(); !it.is_end(); ++it)
        {
            if (reinterpret_cast<uint64_t>(it->get_end()) > regionBegin)
                regionBegin = BYTE_ALIGN_UP(reinterpret_cast<uint64_t>(it->get_end()), PAGE_4KiB);
        }
        MappedRegion region(reinterpret_cast<void*>(regionBegin), reinterpret_cast<uint8_t*>(regionBegin)+size);
        regions.append(region);
        for (auto allocateSpace = regionBegin; allocateSpace < regionBegin + size + PAGE_4KiB; allocateSpace += PAGE_4KiB)
        {
             MemoryManager::instance().alloc_page(VirtualMemoryAllocationRequest(allocateSpace, true, true), *this);
        }
        return region;
    }
}
