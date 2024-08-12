#ifndef PIC_H
#define PIC_H

#include "cpu/io/io.h"

namespace PIC
{
    namespace 
    {
        static constexpr uint16_t MASTER_PIC_CMD_PORT = 0x20;
        static constexpr uint16_t MASTER_PIC_DATA_PORT = 0x21;    
        static constexpr uint16_t SLAVE_PIC_CMD_PORT = 0xA0;
        static constexpr uint16_t SLAVE_PIC_DATA_PORT = 0xA1;
    }

    void send_eoi(uint8_t irq);
    void remap_and_init_to_offsets(uint64_t offset1, uint64_t offset2);
    void set_irq_mask(uint8_t irqBit);
    void clear_irq_mask(uint8_t irqBit);
}

#endif
