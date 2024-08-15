#include "cpu/interrupts/interrupts.h"
#include "memory/MemoryManager.h"

extern void* isr_table[];

namespace CPU {

    InterruptHandler::InterruptHandler(uint8_t interruptNumber)
    {
        m_interruptNumber = interruptNumber;
        InterruptManager::instance().add_interrupt_handler(this, interruptNumber);
        m_isPresentInTable = true;
        if (interruptNumber >= InterruptManager::MAX_EXCEPTIONS_IRQ && interruptNumber < InterruptManager::MAX_PIC_IRQ)
        {
            PIC::clear_irq_mask(interruptNumber - InterruptManager::MAX_EXCEPTIONS_IRQ);
        }
    }

    InterruptHandler::~InterruptHandler()
    {
        if (m_isPresentInTable)
        {
            InterruptManager::instance().remove_interrupt_handler(m_interruptNumber);
        }
    }

    void InterruptManager::initialise()
    {
        uint16_t codeSegmentSelector;
        asm("mov %%CS, %0" : "=r" (codeSegmentSelector));
        m_idtr.set_base(reinterpret_cast<uint64_t>(&m_idt64));
        m_idtr.set_limit(sizeof(InterruptDescriptorTableEntry64) * MAX_INTERRUPTS_IRQ - 1);
        m_codeSegmentSelector = codeSegmentSelector;
        for (int vector = 0; vector <= MAX_PIC_IRQ; vector++)
        {
            InterruptDescriptorTableEntry64* entry = &m_idt64[vector];
            entry->set_offset(reinterpret_cast<uint64_t>(isr_table[vector]));
            entry->set_code_segment_selector(m_codeSegmentSelector);
            entry->set_flags(InterruptDescriptorTableEntry64::PRESENT | InterruptDescriptorTableEntry64::DT64 | InterruptDescriptorTableEntry64::USER);
            //idt_set_entry(i, isr_table[i], 0x8E|0x60); // 0x8C for present, 00 DPL, 64-bit descriptor type
        }
        __asm__ volatile ("lidt %0" : : "m"(m_idtr));
        for (int i = 0; i < MAX_INTERRUPTS_IRQ; i++)
        {
            m_interruptHandlerTable[i] = nullptr;
        }
        enable_interrupts();
    }

    void InterruptManager::add_interrupt_handler(InterruptHandler* handler, uint8_t interrupt)
    {
        if (interrupt >= MAX_EXCEPTIONS_IRQ && interrupt <= MAX_PIC_IRQ)
        {
            PIC::clear_irq_mask(interrupt - MAX_EXCEPTIONS_IRQ);
        }
        m_interruptHandlerTable[interrupt] = handler;
    }

    void InterruptManager::remove_interrupt_handler(uint8_t interrupt)
    {
        if (interrupt >= MAX_EXCEPTIONS_IRQ && interrupt <= MAX_PIC_IRQ)
        {
            PIC::set_irq_mask(interrupt - MAX_EXCEPTIONS_IRQ);
        }
        m_interruptHandlerTable[interrupt] = nullptr;
    }

    void InterruptHandler::update_handler_in_interrupt_table(uint8_t interrupt)
    {
        if (m_isPresentInTable)
        {
            InterruptManager::instance().remove_interrupt_handler(m_interruptNumber);
        }
        m_interruptNumber = interrupt;
        InterruptManager::instance().add_interrupt_handler(this, m_interruptNumber);
        m_isPresentInTable = true;
    }
