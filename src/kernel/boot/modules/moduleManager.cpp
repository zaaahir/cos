#include "boot/modules/moduleManager.h"
#include "memory/MemoryManager.h"
#include "memory/memory.h"
#include "panic.h"
#include "print.h"

namespace Modules
{
    void protect_pages()
    {
        for (auto module = Multiboot::MultibootManager::instance().first(MULTIBOOT_TAG_TYPE_MODULE); !module.is_end(); ++module)
        {
            auto tag = reinterpret_cast<multiboot_tag_module*>(&(*module));
            Memory::MemoryManager::instance().deinit_physical_region(tag->mod_start, tag->mod_end - tag->mod_start);
        }
    }

    void map_virtual()
    {
        for (auto module = Multiboot::MultibootManager::instance().first(MULTIBOOT_TAG_TYPE_MODULE); !module.is_end(); ++module)
        {
            auto tag = reinterpret_cast<multiboot_tag_module*>(&(*module));

            // Modules should be page aligned, panic if they are not. 
            if (!CHECK_ALIGN(tag->mod_start, Memory::PAGE_4KiB) || !CHECK_ALIGN(tag->mod_end, Memory::PAGE_4KiB))
            {
                Kernel::panic("Module is not page aligned!");
            }
            
            // Map modules into virtual memory.
            for (uint64_t i = 0; i<(tag->mod_end-tag->mod_start) + Memory::PAGE_4KiB; i += Memory::PAGE_4KiB)
            {
                auto request = Memory::VirtualMemoryMapRequest(tag->mod_start + i, MODULE_BASE + i);
                request.allowWrite = true;
                Memory::MemoryManager::instance().request_virtual_map(request);
            }
            // TODO: support multiple modules.
        }
    }
}
