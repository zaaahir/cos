#ifndef X86_64_H
#define X86_64_H

#include "types.h"

namespace CPU
{
    const uint64_t INTERRUPT_FLAG = 0x0200;

    static inline uint64_t read_rflags()
    {
        uint64_t rflags;
        asm volatile ("pushfq; popq %0" : "=r" (rflags));
        return rflags;
    }

    static inline uint64_t cli()
    {
        asm volatile ("cli");
    }

    static inline uint64_t sti()
    {
        asm volatile ("sti");
    }

    static inline uint64_t hlt()
    {
        asm volatile ("hlt");
    }
}

#endif
