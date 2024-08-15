#include "event/dispatcher.h"
#include "process/taskManager.h"

namespace Events {

    void EventDispatcher::register_event_sender(char* event, EventSender* sender)
    {
        auto str = FixedLenStringHashContainer();
        strcpy(str.str, event);
        m_eventSenderHashmap.insert(str, sender);
    }

    void EventDispatcher::add_process_to_event_list(EventDescriptorItem item, Common::DoublyLinkedList<EventDescriptorItem>* list)
    {
        list->append(item);
    } 

    void EventDispatcher::add_process_to_listener_event_list(uint64_t ped, uint64_t tid, Common::DoublyLinkedList<EventDescriptorItem>* list)
    {
        add_process_to_event_list({ped, tid}, list);
        Task::TaskManager::instance().get_ed(ped, tid)->last.listenerListElements.append(list);

    } 

    void EventDispatcher::add_process_to_wait_event_list(uint64_t ped, uint64_t tid, Common::DoublyLinkedList<EventDescriptorItem>* list)
    {
        add_process_to_event_list({ped, tid}, list);
        Task::TaskManager::instance().get_ed(ped, tid)->last.waitListElements.append(list);

    }

    void EventDispatcher::remove_process_from_listener_lists(uint64_t ped, uint64_t tid)
    {
        Task::TaskManager::instance().lock();
        auto descriptor = Task::TaskManager::instance().get_ed(ped, tid);
        for (auto it = descriptor->last.listenerListElements.first(); !it.is_end(); ++it)
        {
            for (auto listit = (*it)->first(); !listit.is_end(); ++listit)
            {
                if (listit->tid == tid) { (*it)->remove(listit); break; }
            }
        }
        Task::TaskManager::instance().unlock();
    }

    void EventDispatcher::remove_process_from_wait_lists(uint64_t ped, uint64_t tid)
    {
        Task::TaskManager::instance().lock();
        auto descriptor = Task::TaskManager::instance().get_ed(ped, tid);
        for (auto it = descriptor->last.waitListElements.first(); !it.is_end(); ++it)
        {
            for (auto listit = (*it)->first(); !listit.is_end(); ++listit)
            {
                if (listit->tid == tid) { (*it)->remove(listit); break; }
            }
        }
        Task::TaskManager::instance().unlock();
    }

    void EventDispatcher::wake_processes(Common::DoublyLinkedList<EventDescriptorItem>* list)
    {
        // We need to make a copy of the list as we are removing items from the original.
        auto listCopy = *list;
        for (auto it = listCopy.first(); !it.is_end(); ++it)
        {
            remove_process_from_wait_lists((*it).processEventDescriptor, (*it).tid);
            Task::TaskManager::instance().unblock_task((*it).tid);
        }
    }

    uint64_t EventDispatcher::register_event_listener(uint64_t tid, char* event, void* param)
    { 
        auto str = FixedLenStringHashContainer();
        strcpy(str.str, event);
        
    }
}
