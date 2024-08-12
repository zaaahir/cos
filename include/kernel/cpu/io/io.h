#ifndef IO_H
#define IO_H

#include "types.h"

namespace IO
{
    inline uint8_t in_8(uint16_t port)
    {
        uint8_t ret;
        asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    inline uint16_t in_16(uint16_t port)
    {
        uint16_t ret;
        asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    inline uint32_t in_32(uint16_t port)
    {
        uint32_t ret;
        asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    inline void out_8(uint16_t port, uint8_t value)
    {
        asm volatile ("outb %0, %1" : : "a"(value), "Nd" (port)); 
    }

    inline void out_16(uint16_t port, uint16_t value)
    {
        asm volatile ("outw %0, %1" : : "a"(value), "Nd" (port));
    }

    inline void out_32(uint16_t port, uint32_t value)
    {
        asm volatile ("outl %0, %1" : : "a"(value), "Nd" (port));
    }

    inline void wait()
    {
        out_8(0, 0x80);
    }
}

#endif
