#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include "address.h"
#include "common/doublyLinkedList.h"

namespace Memory
{
    class RegionableVirtualAddressSpace : public VirtualAddressSpace
    {
    public:
        RegionableVirtualAddressSpace(PhysicalAddress topLevelTable) : VirtualAddressSpace(VirtualAddressSpace(topLevelTable)) {}
        RegionableVirtualAddressSpace(VirtualAddressSpace addressSpace) : VirtualAddressSpace(addressSpace) {}
        Common::DoublyLinkedList<MappedRegion> regions;
        MappedRegion get_available_region(uint64_t size);
    };
}

#endif
