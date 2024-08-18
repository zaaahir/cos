#include "process/spinlock.h"
#include "x86_64.h"
#include "print.h"

uint64_t Spinlock::m_cli = 0;
bool Spinlock::m_shouldRestore = false;

// When acquiring spinlocks, interrupts on the local processor must be disabled to avoid a race condition.

void Spinlock::acquire()
{
    // We want to count the number of spinlocks acquired so that we only re-enable interrupts when all are released.
    push_cli();
    while (__atomic_test_and_set(&m_locked, __ATOMIC_SEQ_CST));
}

bool Spinlock::try_acquire()
{
    push_cli();
    if (__atomic_test_and_set(&m_locked, __ATOMIC_SEQ_CST))
    {
        // We could not acquire the spinlock.
        pop_cli();
        return false;
    }
    return true;
}

void Spinlock::release()
{
    __atomic_clear(&m_locked, __ATOMIC_SEQ_CST);
    pop_cli();
}

void Spinlock::push_cli()
{
    uint64_t rflags;
    rflags = CPU::read_rflags();
    CPU::cli();
    // If interrupts are enabled, store this so we can re-enable once we have released all spinlocks.
    if (Spinlock::m_cli++ == 0)
        Spinlock::m_shouldRestore = rflags & CPU::INTERRUPT_FLAG;
}

void Spinlock::pop_cli()
{
    // If we have released all spinlocks and interrupts were previously enabled, re-enable interrupts.
    if (--Spinlock::m_cli == 0 && Spinlock::m_shouldRestore)
        CPU::sti();
}

bool Spinlock::is_acquired()
{
    return m_locked;
}
