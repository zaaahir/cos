#ifndef ADDRESS_H
#define ADDRESS_H

#include "types.h"

namespace Memory
{
    class PhysicalAddress
    {
    public:
        PhysicalAddress() : m_physAddress(0) {}
        PhysicalAddress(uint64_t physicalAddress) : m_physAddress(physicalAddress) {}
        PhysicalAddress(void* physicalAddress) : m_physAddress(reinterpret_cast<uint64_t>(physicalAddress)) {}
        void* get() { return reinterpret_cast<void*>(m_physAddress); }
    private:
        uint64_t m_physAddress;
    };

    class VirtualAddress
    {
    public:
        VirtualAddress(uint64_t virtualAddress) : m_virtualAddress(virtualAddress) {}
        VirtualAddress(void* virtualAddress) : m_virtualAddress(reinterpret_cast<uint64_t>(virtualAddress)) {}
        VirtualAddress(PhysicalAddress physicalAddress) : m_virtualAddress(reinterpret_cast<uint64_t>(physicalAddress.get()) + KERNEL_PHYS_MAP_ADDRESS) {}
        void* get() { return reinterpret_cast<void*>(m_virtualAddress); }
        PhysicalAddress get_low_physical() { return PhysicalAddress(m_virtualAddress - KERNEL_PHYS_MAP_ADDRESS); }
        static constexpr uint64_t KERNEL_PHYS_MAP_ADDRESS = 0xFFFFFF8000000000;
    private:
        uint64_t m_virtualAddress;
    };
}

#endif
