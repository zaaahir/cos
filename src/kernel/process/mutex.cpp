#include "process/mutex.h"

Mutex::Mutex() : m_acquired(false) {}

void Mutex::acquire()
{
    if (!Task::TaskManager::is_executing()) { return; }
    Task::TaskManager::instance().lock();

    if (m_acquired)
    {
        // If the mutex is already acquired, we add the current task to a wait list and sleep to avoid busy-waiting.
        m_waitingTasks.append(Task::TaskManager::instance().get_current_task());
        Task::TaskManager::instance().block_task();
    }
    else
    {
        m_acquired = true;
        Task::TaskManager::instance().unlock();
    }
}

void Mutex::release()
{
    if (!Task::TaskManager::is_executing()) { return; }
    
    Task::TaskManager::instance().lock();
    auto it = m_waitingTasks.first();
    if (it.is_end())
    {
        // There are no tasks trying to acquire the mutex, it can be released.
        m_acquired = false;
    }
    else
    {
        // Wake the first task on the wait-list who now holds the mutex
        Task::TaskManager::instance().unblock_task(*it);
        m_waitingTasks.remove(it);
    }
    Task::TaskManager::instance().unlock();
}
