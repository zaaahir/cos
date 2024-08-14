#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "event/sender.h"
#include "event/dispatcher.h"
#include "cpu/io/io.h"
#include "cpu/interrupts/interrupts.h"

// FIXME: I think there's an issue which is going to pop up here, but I don't really know. Theoretically this should be enough? Refer back to later, maybe ask someone. 

namespace Drivers 
{
    class KeyboardDriver : Events::EventSender, CPU::InterruptHandler
    {
    public:
        KeyboardDriver();
        ~KeyboardDriver() = default;
        void register_event_listener(uint64_t ped, uint64_t tid);
        void block_event_listen(uint64_t ped, uint64_t tid);
        void irq_handler();
        static KeyboardDriver* instance();
    private:
        static KeyboardDriver* m_self;
        Common::DoublyLinkedList<Events::EventDescriptorItem> m_listenqueue;
        Common::DoublyLinkedList<Events::EventDescriptorItem> m_waitqueue;
    };
}

#endif
