#include "cpu/interrupts/pic.h"
#include "print.h"

namespace PIC
{
    void remap_and_init_to_offsets(uint64_t offset1, uint64_t offset2)
    {
        // read in masks from Data Bus as ICW1 resets masks
        uint8_t masterMask = IO::in_8(MASTER_PIC_DATA_PORT); 
        // the masks can be read from Data Bus whenever RD is active and A0 = 1
        uint8_t slaveMask = IO::in_8(SLAVE_PIC_DATA_PORT); 
        // issue ICW1, 0x10 | 0x01 (init | ICW4)
        IO::out_8(MASTER_PIC_CMD_PORT, 0x10 | 0x01); 
        IO::wait();
        // ICW1
        IO::out_8(SLAVE_PIC_CMD_PORT, 0x10 | 0x01); 
        IO::wait();
        // master PIC offset
        IO::out_8(MASTER_PIC_DATA_PORT, offset1);  
        IO::wait();
        // slave PIC offset
        IO::out_8(SLAVE_PIC_DATA_PORT, offset2); 
        IO::wait();
        // issue ICW3, master PIC has slave on IR 2
        IO::out_8(MASTER_PIC_DATA_PORT, 0x4); 
        IO::wait();
        // ICW3, slave ID of 2
        IO::out_8(SLAVE_PIC_DATA_PORT, 0x2); 
        IO::wait();
        // ICW4, 8086 mode
        IO::out_8(MASTER_PIC_DATA_PORT, 0x1); 
        IO::wait();
        // ICW4, 8086 mode
        IO::out_8(SLAVE_PIC_DATA_PORT, 0x1);
        IO::wait();
        //restore masks
        IO::out_8(MASTER_PIC_DATA_PORT, 0xFB); 
        IO::wait();
        IO::out_8(SLAVE_PIC_DATA_PORT, 0xFF);
    }

    void send_eoi(uint8_t irq)
    {
        if (irq >= 8)
        {
            // send End of Interrupt (EOI) to slave PIC (if issuer)
            IO::out_8(SLAVE_PIC_CMD_PORT, 0x20); 
        }
        // send EOI to master PIC
        IO::out_8(MASTER_PIC_CMD_PORT, 0x20); 
    }

    void set_irq_mask(uint8_t irqLine)
    {
        uint16_t port = irqLine < 8 ? MASTER_PIC_DATA_PORT : SLAVE_PIC_DATA_PORT;
        uint8_t imr;
        if (irqLine >= 8)
        {
            irqLine -= 8;
        }
        imr = IO::in_8(port) | (1 << irqLine);
        IO::out_8(port, imr);
    }

    void clear_irq_mask(uint8_t irqLine)
    {
        uint16_t port = irqLine < 8 ? MASTER_PIC_DATA_PORT : SLAVE_PIC_DATA_PORT;
        uint8_t imr;
        if (irqLine >= 8)
        {
            irqLine -= 8;
        }
        imr = IO::in_8(port) & ~(1 << irqLine);
        IO::out_8(port, imr);
    }
}
