#ifndef PIT_H
#define PIT_H

#include "cpu/io/io.h"
#include "cpu/interrupts/interrupts.h"

#define CHANNEL_0_DATA          0x40
#define CHANNEL_1_DATA          0x41
#define CHANNEL_2_DATA          0x42
#define COMMAND_REGISTER        0x43
#define TIMER_CHANNEL_0         0b00000000
#define TIMER_CHANNEL_1         0b01000000
#define TIMER_CHANNEL_2         0b10000000
#define ACCESS_MODE_LATCH_COUNT 0b00000000
#define ACCESS_MODE_LO          0b00010000
#define ACCESS_MODE_HI          0b00100000
#define ACCESS_MODE_LOHI        0b00110000
#define OPERATING_MODE_INTR     0b00000000
#define OPERATING_MODE_RG       0b00000100
#define BCD4DIGIT_MODE          0b00000001
#define BINARY16BIT_MODE        0b00000000

#define INIT_FREQUENCY          100

namespace CPU {
    class PIT {
    public:
        void initialise();
        void set_frequency(uint64_t frequency);
        static void tick();
        static PIT& instance()
        {
            static PIT instance;
            return instance;
        }
        PIT(PIT const&) = delete;
        PIT& operator=(PIT const&) = delete;
        static uint64_t time_since_boot() { return PIT::instance().m_timeSinceBootNano; }
        
    private:
        PIT() {}
        void reload_count();
        uint64_t m_freq;
        uint64_t m_timeSinceBootNano;
        uint64_t m_ticks;
    };
}

#endif
