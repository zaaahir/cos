#include "loader/elf.h"

namespace Loader
{
    /*
        This is a loader for ELF-64 executables - refer to the ELF-64 Specification: https://uclibc.org/docs/elf-64-gen.pdf
    */
   extern "C" void start_userspace_task_iretq(void* entryPoint, void* kstack);

    bool check_elf_header_ident(Elf64_Ehdr* header)
    {
        if (!header) { return false; }
        if (header->e_ident[EI_MAG0] != ELFMAG0 || header->e_ident[EI_MAG1] != ELFMAG1
            || header->e_ident[EI_MAG2] != ELFMAG2 || header->e_ident[EI_MAG3] != ELFMAG3) { return false; }
        return true;
    }

    bool check_elf_header_support(Elf64_Ehdr* header)
    {
        if (header->e_ident[EI_CLASS] != ELFCLASS64) { return false; }
        if (header->e_ident[EI_DATA] != ELFDATA2LSB) { return false; }
        if (header->e_machine != EM_AMD64) { return false; }
        if (header->e_ident[EI_VERSION] != EV_CURRENT) { return false; }
        if (header->e_type != ET_REL && header->e_type != ET_EXEC) { return false; }
        return true;
    }

    Elf64_Shdr* get_section_header(Elf64_Ehdr* header)
    {
        return reinterpret_cast<Elf64_Shdr*>(reinterpret_cast<uint64_t>(header) + header->e_shoff);
    }

    Elf64_Shdr* get_section(Elf64_Ehdr* header, int index)
    {
        return &get_section_header(header)[index];
    }

    void* get_string_table(Elf64_Ehdr* header)
    {
        return reinterpret_cast<uint8_t*>(header) + get_section(header, header->e_shstrndx)->sh_offset;
    }

    char* lookup_string_in_table(Elf64_Ehdr* header, int offset)
    {
        char* table = static_cast<char*>(get_string_table(header));
        return table + offset;
    }

    void prepare_elf(char* path, Task::Task* task)
    {
        // Load the ELF file into memory.
        auto fd = Filesystem::VirtualFilesystemManager::instance().open_file(path, 0);
        auto elfFile = reinterpret_cast<Elf64_Ehdr*>(kmalloc(Filesystem::VirtualFilesystemManager::instance().filelen(fd), 0));
        Filesystem::VirtualFilesystemManager::instance().read(fd, 0, Filesystem::VirtualFilesystemManager::instance().filelen(fd), elfFile);
        if (!check_elf_header_ident(elfFile) || !check_elf_header_support(elfFile)) { return; }

        // Iterate through ELF headers.
        for (uint8_t* idx = (uint8_t*)(((uint64_t)elfFile)+elfFile->e_shoff); (uint64_t)idx < ((uint64_t)elfFile)+elfFile->e_shoff+elfFile->e_shentsize * elfFile->e_shnum; idx += elfFile->e_shentsize)
        {
            auto sectionHeader = (Elf64_Shdr*)idx;
            // If the section has a virtual address, we need to load it in.
            if (sectionHeader->sh_addr)
            {
                // Allocate pages for the section.
                task->get_tlTable().regions.append(Memory::MappedRegion((void*)BYTE_ALIGN_DOWN(sectionHeader->sh_addr, Memory::PAGE_4KiB), (void*)BYTE_ALIGN_DOWN(sectionHeader->sh_addr, Memory::PAGE_4KiB) + sectionHeader->sh_size + Memory::PAGE_4KiB*2));
                for (uint8_t* page = (uint8_t*)(BYTE_ALIGN_DOWN(sectionHeader->sh_addr, Memory::PAGE_4KiB)); (uint64_t)page < BYTE_ALIGN_DOWN(sectionHeader->sh_addr, Memory::PAGE_4KiB) + sectionHeader->sh_size + Memory::PAGE_4KiB*2; page+= Memory::PAGE_4KiB)
                {
                    if (!Memory::MemoryManager::instance().get_physical_address(page, task->get_tlTable()).get())
                    {
                        auto request = Memory::VirtualMemoryAllocationRequest(page);
                        request.allowUserAccess = true;
                        request.allowWrite = true;
                        Memory::MemoryManager::instance().alloc_page(request, task->get_tlTable());
                    }
                }
            }
        }
    }
}
