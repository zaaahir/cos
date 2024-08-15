#include "boot/multibootManager.h"
#include "print.h"

namespace Multiboot
{
    MultibootTagTypeIterator& MultibootTagTypeIterator::operator++()
    {
        if (m_tag->type == MULTIBOOT_TAG_TYPE_END) { return *this; }

        do
        {
            // Multiboot2 tags are padded out so that each consecutive tag starts on a 8-byte boundary.
            m_tag = reinterpret_cast<multiboot_tag*>(reinterpret_cast<uint8_t*>(m_tag) + BYTE_ALIGN_UP(m_tag->size, 8));
        }
        while (m_tag->type != m_desiredTag && m_tag->type != MULTIBOOT_TAG_TYPE_END);

        return *this;
    }

    MultibootTagTypeIterator MultibootManager::first(uint32_t tagType)
    {
        MultibootTagTypeIterator it(m_structure->tags, tagType);
        // Make sure that we return a desired tag.
        if (it.m_tag->type != it.m_desiredTag) { ++it; }
        return it;
    }
}
