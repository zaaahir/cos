#ifndef MUTEX_H
#define MUTEX_H

#include "process/taskManager.h"

class Mutex
{
    Common::DoublyLinkedList<Task::TaskID> m_waitingTasks;
    bool m_acquired;
public:
    Mutex();
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    void acquire();
    void release();
};

#endif
