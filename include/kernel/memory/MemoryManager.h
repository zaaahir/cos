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

    class GenericEntry
    {
        uint64_t m_entry = 0;
    public:
        enum Flag : uint64_t
        {
            PRESENT = 0x1,
            WRITEABLE = 0x2,
            USER_ACCESS = 0x4,
            PAGE_SIZE = 0x80
        };
        bool get_flag(Flag flag) { return m_entry & flag; }
        void set_flag(Flag flag) { m_entry |= flag; }
        void clear_flag(Flag flag) { m_entry & (~flag); }
        void set_frame(PhysicalAddress frame)
        {
            m_entry = reinterpret_cast<uint64_t>(frame.get()) | (m_entry & 0xFFF);
        }

        void clear() { m_entry = 0; }

        bool is_present() { return get_flag(PRESENT); }

        PhysicalAddress get_frame()
        {
            return PhysicalAddress(m_entry & ~0xFFF);
        }

        void set_access_flags(VirtualMemoryMapRequest request);

        void prepare_table_for_entry(VirtualMemoryMapRequest request);

        void prepare_page_for_entry(VirtualMemoryMapRequest request);

        void free_table();
    };

    class GenericTable
    {
    public:
        bool is_table_empty()
        {
            uint64_t* p = (uint64_t*)m_entries;
            uint64_t sz = ENTRIES_PER_TABLE;
            while (sz--)
                if (*p++ != 0) { return false; }
            return true;
        }
    protected:
        static constexpr int ENTRIES_PER_TABLE = 512;
        GenericEntry m_entries[ENTRIES_PER_TABLE] = {};
    };

    class PML4Table : public GenericTable
    {
    public:
        void request_virtual_map(VirtualMemoryMapRequest request);
        void request_virtual_unmap(VirtualMemoryUnmapRequest request);
        PhysicalAddress get_physical_address(VirtualAddress virtualAddress);
    private:
        GenericEntry* get_entry(VirtualAddress virtualAddress)
        {
            return &m_entries[(reinterpret_cast<uint64_t>(virtualAddress.get()) >> 39) & 0x1FF];
        }
    } __attribute__((packed));

    class PageDirectoryPointerTable : public GenericTable
    {
    public:
        void request_virtual_map(VirtualMemoryMapRequest request);
        bool request_virtual_unmap(VirtualMemoryUnmapRequest request);
        PhysicalAddress get_physical_address(VirtualAddress virtualAddress);
    private:
        GenericEntry* get_entry(VirtualAddress virtualAddress)
        {
            return &m_entries[(reinterpret_cast<uint64_t>(virtualAddress.get()) >> 30) & 0x1FF];
        }
    } __attribute__((packed));
