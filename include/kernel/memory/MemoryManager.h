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

    class PageDirectoryTable : public GenericTable
    {
    public:
        void request_virtual_map(VirtualMemoryMapRequest request);
        bool request_virtual_unmap(VirtualMemoryUnmapRequest request);
        PhysicalAddress get_physical_address(VirtualAddress virtualAddress);
    private:
        GenericEntry* get_entry(VirtualAddress virtualAddress)
        {
            return &m_entries[(reinterpret_cast<uint64_t>(virtualAddress.get()) >> 21) & 0x1FF];
        }
    } __attribute__((packed));

    class PageTable : public GenericTable
    {
    public:
        void request_virtual_map(VirtualMemoryMapRequest request);
        bool request_virtual_unmap(VirtualMemoryUnmapRequest request);
        PhysicalAddress get_physical_address(VirtualAddress virtualAddress);
    private:
        GenericEntry* get_entry(VirtualAddress virtualAddress)
        {
            return &m_entries[(reinterpret_cast<uint64_t>(virtualAddress.get()) >> 12) & 0x1FF];
        }
    } __attribute__((packed));

    class MemoryManager
    {
        friend class PML4Table;
    public:
        // Protect certain regions from being overwritten by allocator.
        void protect_physical_regions(PhysicalAddress kernelEnd);
        // Mark a region of memory as free so the physical allocator can use them
        void init_physical_region(PhysicalAddress base, uint64_t size);
        // Mark a region of memory as used so the physical allocator does not overwrite them.
        void deinit_physical_region(PhysicalAddress base, uint64_t size);
        // Set up new kernel page tables.
        void remap_pages();
        // Allocate a 4KiB physical page.
        PhysicalAddress alloc_physical_block();
        // Frees a 4KiB physical page.
        void free_physical_block(PhysicalAddress block);
        // Map a virtual page in an address space.
        void request_virtual_map(VirtualMemoryMapRequest request, VirtualAddressSpace& addressSpace = instance().get_main_address_space());
        // Unmap a virtual page in an address space.
        void request_virtual_unmap(VirtualMemoryUnmapRequest request, VirtualAddressSpace& addressSpace = instance().get_main_address_space());
        // Allocate a virtual 4KiB page.
        void alloc_page(VirtualMemoryAllocationRequest request, VirtualAddressSpace& addressSpace = instance().get_main_address_space());
        // Free an allocated virtual 4KiB page.
        void free_page(VirtualMemoryFreeRequest request, VirtualAddressSpace& addressSpace = instance().get_main_address_space());
        // Get physical address that a page-aligned virtual address is mapped to.
        PhysicalAddress get_physical_address(VirtualAddress virtualAddress, VirtualAddressSpace& addressSpace = instance().get_main_address_space());

        VirtualAddressSpace create_virtual_address_space()
        {
            auto physicalAddress = alloc_physical_block();
            auto addressSpace = VirtualAddressSpace(physicalAddress);
            memset(VirtualAddress(physicalAddress).get(), 0, sizeof(PML4Table));
            for (uint64_t physicalAddress = 0; physicalAddress < 510 * PAGE_1GiB; physicalAddress += PAGE_1GiB)
            {
                auto request = VirtualMemoryMapRequest(PhysicalAddress(physicalAddress), VirtualAddress(PhysicalAddress(physicalAddress)));
                request.allowWrite = true;
                request.pageSize = PAGE_1GiB;
                request_virtual_map(request, addressSpace);
            }
            return addressSpace;
        }

        void destroy_virtual_address_space(VirtualAddressSpace& addressSpace)
        {
            free_physical_block(addressSpace.get_physical_address().get());
        }
