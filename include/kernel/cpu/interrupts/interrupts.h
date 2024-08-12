#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include "types.h"
#include "cpu/interrupts/pic.h"
#include "print.h"

namespace CPU {

    class InterruptHandler
    {
    public:
        InterruptHandler() = default;
        InterruptHandler(uint8_t interruptNumber);
        // Add handler to interrupt table to be called when interruptNumber vector is received.
        void update_handler_in_interrupt_table(uint8_t interruptNumber);
        ~InterruptHandler();
        virtual void irq_handler() = 0;
    private:
        uint8_t m_interruptNumber;
        bool m_isPresentInTable = false;
        
    };
}
#endif
