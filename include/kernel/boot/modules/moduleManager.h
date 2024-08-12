#ifndef MODULES_H
#define MODULES_H

#include "boot/multibootManager.h"

namespace Modules
{
    static constexpr uint64_t MODULE_BASE = 0xFFFFFFFFA0000000;
    // Map modules into virtual address space.
    void protect_pages();
    // Ensure modules are not overwritten by the physical allocator.
    void map_virtual();
}
#endif
