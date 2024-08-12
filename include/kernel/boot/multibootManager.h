#ifndef MULTIBOOTMANAGER_H
#define MULTIBOOTMANAGER_H
#include "types.h"
#include "multiboot2.h"
#include "memory/memory.h"
#include "print.h"

namespace Multiboot
{
    struct multiboot_info
    {
        multiboot_uint32_t total_size;
        multiboot_uint32_t reserved;
        multiboot_tag tags[];
    }__attribute__((packed));

    class MultibootTagTypeIterator
    {
        friend class MultibootManager;
    public:
        multiboot_tag& operator*() { return *m_tag; }
        multiboot_tag* operator->() { return m_tag; }
        MultibootTagTypeIterator& operator++();
        bool is_end() { return m_tag->type == MULTIBOOT_TAG_TYPE_END; }
    private:
        MultibootTagTypeIterator(multiboot_tag* tag, uint32_t desiredTag) : m_tag(tag), m_desiredTag(desiredTag) {}
        multiboot_tag* m_tag;
        uint32_t m_desiredTag;
    };

    class MultibootManager
    {
    public:
        static MultibootManager& instance()
        {
            static MultibootManager instance;
            return instance;
        }
        MultibootManager(MultibootManager const&) = delete;
        void operator=(MultibootManager const&) = delete;
        
        MultibootTagTypeIterator first(uint32_t tagType);
        multiboot_info* get_structure() { return m_structure; }
        void set_structure(multiboot_info* structure) { m_structure = structure; }
    private:
        MultibootManager() {}
        multiboot_info* m_structure;
    };
}

#endif
