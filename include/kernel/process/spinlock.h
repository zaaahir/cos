#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

class Spinlock
{
private:
    static uint64_t m_cli;
    static bool m_shouldRestore;
    uint64_t m_locked = 0;
    void pop_cli();
    void push_cli();
public:
    void acquire();
    void release();
    bool try_acquire();
    bool is_acquired();
};

#endif
