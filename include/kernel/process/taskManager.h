#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "process/task.h"
#include "process/scheduler.h"
#include "common/priorityQueue.h"
#include "x86_64.h"

namespace Task
{
    static constexpr uint64_t START_OF_PROCESS_KSTACKS = 0xFFFFCF8000000000;

    class TaskManager
    {
        friend class Task;
        friend class Mutex;
    public:
        static TaskManager& instance()
        {
            static TaskManager instance;
            return instance;
        }
        void run();
        void add_task(Task task);
        void block_task();
        void unblock_task(TaskID taskID);
        Task create_new_task(SharableTaskResource<Memory::RegionableVirtualAddressSpace> tlTable, SharableTaskResource<FileDescriptorTable> openFileDescriptors,
                    SharableTaskResource<EventDescriptorTable> openEventDescriptors, uint64_t taskPriority = 512);
        Task create_new_kernel_task(uint64_t taskPriority = 512);
        // Add an event descriptor to a task.
        uint64_t add_ed(void* eventSender, void* param, TaskID task);
        // Remove an event descriptor from a task.
        void remove_ed(uint64_t ed, TaskID task);
        void lock() { m_spinlock.acquire(); }
        void unlock() { m_spinlock.release(); }
        static uint64_t get_current_task() { return TaskManager::instance().m_scheduler->get_current_task(); }
        Task* get_current_task_pointer() { return m_scheduler->get_task_from_tid(m_scheduler->get_current_task()); }
        Common::DoublyLinkedListIterator<Common::Pair<uint64_t, Events::EventDescriptor>> get_ed(uint64_t ped, TaskID tid);
        // Called by task switching assembly routine
        static void pit_tick();
        static void refresh();
        static uint64_t* get_safe_stack();
        static void save_kstack(void* kstack);
        static void save_kstack_with_tid(void* kstack, TaskID tid);
        static uint64_t* get_current_stack();
        static bool is_executing();
        static bool is_spinlock_acquired(); // need to check current cpu
        static bool is_scheduler_changing_task();
        static void terminate_task();
        // Put current task to sleep until a later time.
        void sleep_until(uint64_t time);
        // Put current task to sleep for a duration.
        static void sleep_for(uint64_t duration);
    private:
        TaskManager();
        Scheduler* m_scheduler;
        static bool m_isActive;
        bool m_schedulerChangingTask = false;
        uint64_t m_numberOfTasks;
        void switch_to_task(Task* task);
        Spinlock m_spinlock;
        struct SleepingTask
        {
            TaskID task;
            uint64_t wakeTime;
            friend bool operator>(SleepingTask const& lhs, SleepingTask const& rhs)
            {
                return lhs.wakeTime < rhs.wakeTime;
            }
        };
        Common::PriorityQueue<SleepingTask> m_sleepingTasks;
    };
}

#endif
