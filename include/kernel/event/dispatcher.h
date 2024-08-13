#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "event/sender.h"
#include "common/doublyLinkedList.h"
#include "common/hashmap.h"
#include "libc/string.h"

namespace Task { class Task; }

namespace Events {

    struct EventMessage
    {
        void* message;
    };
    
    struct EventDescriptor;

    struct EventDescriptorItem
    {
        uint64_t processEventDescriptor;
        uint64_t tid;
    };

    struct EventDescriptor
    {
        uint64_t ed;
        void* eventSender = nullptr;
        // The param is a free field that the EventSender can use.
        void* param = nullptr;
        void* process = nullptr;
        Common::DoublyLinkedList<EventMessage> queue;
        Common::DoublyLinkedList<Common::DoublyLinkedList<EventDescriptorItem>*> waitListElements;
    };

    struct FixedLenStringHashContainer
    {
        char str[180];
        friend bool operator==(const FixedLenStringHashContainer& lhs, const FixedLenStringHashContainer& rhs)
// not sure how i'm going to implement
    };

    class EventDispatcher
    {
    public:
        static EventDispatcher& instance()
        {
            static EventDispatcher instance;
            return instance;
        }
        EventDispatcher(EventDispatcher const&) = delete;
        EventDispatcher& operator=(EventDispatcher const&) = delete;

    private:
        EventDispatcher() {}
        Common::Hashmap<FixedLenStringHashContainer, EventSender*> m_eventSenderHashmap;
        void dispatch_event(EventMessage event, EventDescriptorItem processed);
        void add_process_to_event_list(EventDescriptorItem item, Common::DoublyLinkedList<EventDescriptorItem>* list);
        void remove_process_from_wait_lists(uint64_t ped, uint64_t tid);
    };
}
#endif
