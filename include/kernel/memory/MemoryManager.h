#ifndef PMM_H
#define PMM_H
#include "types.h"
#include "boot/multibootManager.h"
#include "print.h"
#include "memory/address.h"

#define KERNEL_V_BASE 0xFFFFFF8000000000

namespace Memory {

    // FIXME: when we map, fail if we are overwritting a preexisting map!

    enum PageSize : uint64_t
    {
        PAGE_4KiB = 0x1000,
        PAGE_2MiB = 0x200000,
        PAGE_1GiB = 0x40000000
    };

    class MappedRegion
    {
    public:
        MappedRegion(void* start, void* end) : m_start(start), m_end(end) {}
        void* get_start() { return m_start; }
        void* get_end() { return m_end; }
    private:
        void* m_start;
        void* m_end;
    };

    class MemoryManager;

    class VirtualAddressSpace
    {
    friend class MemoryManager;
    public:
        VirtualAddressSpace(PhysicalAddress topLevelTable) : m_physicalAddress(topLevelTable) {}
        PhysicalAddress get_physical_address() { return m_physicalAddress; }
    protected:
        PhysicalAddress m_physicalAddress;
    };

    struct VirtualMemoryAllocationRequest
    {
        bool allowWrite = false;
        bool allowUserAccess = false;
        VirtualAddress virtualAddress;
        VirtualMemoryAllocationRequest(VirtualAddress virtualAddress, bool allowWrite = false, bool allowUserAccess = false) : virtualAddress(virtualAddress), allowWrite(allowWrite), allowUserAccess(allowUserAccess) {}
    };

    struct VirtualMemoryFreeRequest
    {
        VirtualAddress virtualAddress;
        VirtualMemoryFreeRequest(VirtualAddress virtualAddress) : virtualAddress(virtualAddress) {}
    };

    struct VirtualMemoryMapRequest
    {
        bool allowWrite = false;
        bool allowUserAccess = false;
        PageSize pageSize = PAGE_4KiB;
        PhysicalAddress physicalAddress;
        VirtualAddress virtualAddress;
        VirtualMemoryMapRequest(PhysicalAddress physicalAddress, VirtualAddress virtualAddress, bool allowWrite = false, bool allowUserAccess = false) : physicalAddress(physicalAddress), virtualAddress(virtualAddress), allowWrite(allowWrite), allowUserAccess(allowUserAccess) {}
        VirtualMemoryMapRequest(VirtualMemoryAllocationRequest request, PhysicalAddress physicalAddress) : allowWrite(request.allowWrite), allowUserAccess(request.allowUserAccess),
                                                                                                            physicalAddress(physicalAddress), virtualAddress(request.virtualAddress) {}
    };

    struct VirtualMemoryUnmapRequest
    {
        VirtualAddress virtualAddress;
        PageSize pageSize = PAGE_4KiB;
        VirtualMemoryUnmapRequest(VirtualAddress virtualAddress) : virtualAddress(virtualAddress) {}
        VirtualMemoryUnmapRequest(VirtualMemoryFreeRequest request) : virtualAddress(request.virtualAddress) {}
    };
