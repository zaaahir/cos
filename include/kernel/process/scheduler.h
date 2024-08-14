#ifndef NSCHEDULER_H
#define NSCHEDULER_H

#include "process/task.h"
#include "process/spinlock.h"

namespace Task
{
    class Scheduler
    {
    public:
        virtual void add_task(Task task) = 0;
        virtual void remove_task(TaskID tid) = 0;
        virtual void move_to_next_task(Spinlock& taskSpinlock) = 0;
        virtual TaskID get_current_task() = 0;
        virtual void block_task() = 0;
        virtual void unblock_task(TaskID tid) = 0;
        virtual Task* get_task_from_tid(TaskID tid) = 0;
    };
}

#endif
