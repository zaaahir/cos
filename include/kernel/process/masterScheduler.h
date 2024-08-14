#ifndef MASTERSCHEDULER_H
#define MASTERSCHEDULER_H

#include "process/task.h"
#include "process/scheduler.h"
#include "common/priorityQueue.h"

#define VARFREQ_CIRCULAR_BUFFER_SIZE 256

namespace Task
{
    struct TaskPriorityItem
    {
        TaskID taskID;
        uint64_t priority;
        friend bool operator>(TaskPriorityItem const& lhs, TaskPriorityItem const& rhs)
        {
            return lhs.priority < rhs.priority;
        }
    };

    struct MasterSchedulerTaskInfo
    {
        uint64_t queue = 0;
        uint64_t priority = 0;
    };

    class MasterScheduler : public Scheduler
    {
    public:
        void add_task(Task task);
        void remove_task(TaskID tid);
        void move_to_next_task(Spinlock& taskSpinlock);
        TaskID get_current_task();
        void block_task();
        void unblock_task(TaskID tid);
        Task* get_task_from_tid(TaskID tid);
    private:
        // The VariableFrequencyBuffer maintains tasks in a circular buffer, with lower priority tasks occuring more sparsely.
        class VariableFrequencyBuffer
        {
        private:
            Common::DoublyLinkedList<TaskID> buffer[VARFREQ_CIRCULAR_BUFFER_SIZE];
            uint64_t m_currentList = 0;
            uint64_t m_size = 0;
        public:
            VariableFrequencyBuffer()
            {
                for (int i = 0; i++; i < VARFREQ_CIRCULAR_BUFFER_SIZE)
                {
                    buffer[i] = Common::DoublyLinkedList<TaskID>();
                }
            }
            void add(TaskID tid, uint64_t priority)
            {
                buffer[(m_currentList + priority) % VARFREQ_CIRCULAR_BUFFER_SIZE].append(tid);
                m_size++;
            }
            TaskID get_next()
            {
                if (!m_size) { return 0; }
                
                // Keep moving to next task list in circular buffer until we have a non-empty task list.
                while (!buffer[m_currentList].size())
                {
                    m_currentList++;
                    m_currentList %= VARFREQ_CIRCULAR_BUFFER_SIZE;
                }
                // Remove and return the first task on the list.
                auto ret = *buffer[m_currentList].first();
                buffer[m_currentList].remove(buffer[m_currentList].first());
                m_size--;
                return ret;
            }
            uint64_t size() { return m_size; }
        };

        TaskID get_next_task();
        Common::Hashmap<TaskID, Task> m_taskHashmap;
        TaskID m_currentTask;
        Common::PriorityQueue<TaskPriorityItem> m_queue0;
        Common::PriorityQueue<TaskPriorityItem> m_queue1;
        VariableFrequencyBuffer m_queue2;
        void add_task_to_queue(TaskID task);
    };
}

#endif
