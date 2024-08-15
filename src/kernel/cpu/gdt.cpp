#include "cpu/gdt.h"
#include "print.h"
namespace CPU {

    namespace
    {
        static void* m_globalDescriptorTable;
    }

    void set_global_descriptor_table(void* globalDescriptorTable)
    {
        m_globalDescriptorTable = globalDescriptorTable;
    }

    void set_global_descriptor_table_entry(GlobalDescriptorTableEntry32 entry, int entryNum)
    {
        static_cast<GlobalDescriptorTableEntry32*>(m_globalDescriptorTable)[entryNum] = entry;
    }

    void set_global_descriptor_table_entry(GlobalDescriptorTableEntry64 entry, int entryNum)
    {
        *reinterpret_cast<GlobalDescriptorTableEntry64*>(static_cast<GlobalDescriptorTableEntry32*>(m_globalDescriptorTable) + entryNum) = entry;
    }
}
