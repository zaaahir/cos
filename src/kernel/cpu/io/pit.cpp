#include "cpu/io/pit.h"
#include "print.h"
#include "cpu/interrupts/interrupts.h"

namespace CPU {

    void PIT::initialise()
    {
        m_freq = INIT_FREQUENCY;
        reload_count();
        set_frequency(INIT_FREQUENCY);
        PIC::clear_irq_mask(0);
    }

    void PIT::reload_count()
    {
        // Reset count
        IO::out_8(COMMAND_REGISTER, TIMER_CHANNEL_0 | ACCESS_MODE_LOHI | OPERATING_MODE_RG | BINARY16BIT_MODE);
    }

    void PIT::set_frequency(uint64_t frequency)
    {
        uint16_t value = ((uint64_t)1193182/(uint64_t)frequency);
        IO::out_8(CHANNEL_0_DATA, value & 0xFF);
        IO::out_8(CHANNEL_0_DATA, (value >> 8)&0xFF);
    }

    void PIT::tick()
    {
        // 100 Hz, T = 10ms
        PIT::instance().m_timeSinceBootNano += 10; 
    }
}
