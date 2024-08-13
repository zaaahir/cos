#ifndef SENDER_H
#define SENDER_H

#include "types.h"

namespace Events
{
    struct EventDescriptor;
    class EventSender
    {
    public:
        virtual void register_event_listener(uint64_t ped, uint64_t tid) = 0;
        virtual void block_event_listen(uint64_t ped, uint64_t tid) = 0;
    };
}

#endif
