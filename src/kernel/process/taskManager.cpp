#include "process/task.h"
#include "process/taskManager.h"
#include "cpu/tss.h"
#include "process/roundRobinScheduler.h"
#include "memory/AddressSpace.h"
#include "process/masterScheduler.h"
#include "x86_64.h"
#include "memory/MemoryManager.h"
#include "cpu/io/pit.h"

namespace Task
{
    extern "C" void enable_syscall_sysret();
    extern "C" void start_kernel_task_iretq(void* entryPoint, void* kstack);
    extern "C" void return_to_task(void* tlTable);
    extern "C" void send_pit_eoi();
    extern "C" void block_task_and_refresh(uint64_t tid);

    bool TaskManager::m_isActive = false;

    TaskManager::TaskManager()
    {
        enable_syscall_sysret();
        Memory::MemoryManager::instance().alloc_page(Memory::VirtualMemoryAllocationRequest(Memory::VirtualAddress(START_OF_PROCESS_KSTACKS), true));
        CPU::update_kernel_stack_pointer(get_safe_stack());
        CPU::refresh_task_state_segment();
        m_scheduler = new MasterScheduler();
    }

    Task TaskManager::create_new_task(SharableTaskResource<Memory::RegionableVirtualAddressSpace> tlTable, SharableTaskResource<FileDescriptorTable> openFileDescriptors,
                    SharableTaskResource<EventDescriptorTable> openEventDescriptors, uint64_t taskPriority)
    {
        auto task = Task(tlTable, openFileDescriptors, openEventDescriptors);
        task.set_task_id(++m_numberOfTasks);
        // Queue 0: 0-255
        // Queue 1: 256-511
        // Queue 2: 512-767
        task.set_priority(taskPriority);

        // At the moment, task stacks are allocated contiguously so we can allocate the stack at a position based on the task id.
        auto stackBottom = (START_OF_PROCESS_KSTACKS + Memory::PAGE_4KiB * (task.get_task_id()));
        Memory::MemoryManager::instance().alloc_page(Memory::VirtualMemoryAllocationRequest(Memory::VirtualAddress(stackBottom), true));

        // We need to map a safe kernel stack and the task's kernel stack into the task's address space.
        Memory::MemoryManager::instance().request_virtual_map(Memory::VirtualMemoryMapRequest(Memory::MemoryManager::instance().get_physical_address(START_OF_PROCESS_KSTACKS), Memory::VirtualAddress(START_OF_PROCESS_KSTACKS), true), task.get_tlTable());
        Memory::MemoryManager::instance().request_virtual_map(Memory::VirtualMemoryMapRequest(Memory::MemoryManager::instance().get_physical_address(stackBottom), Memory::VirtualAddress(stackBottom), true), task.get_tlTable());

        task.set_kstack(reinterpret_cast<void*>(stackBottom + Memory::PAGE_4KiB));
        return task;
    }

    Task TaskManager::create_new_kernel_task(uint64_t taskPriority)
    {
        return create_new_task(SharableTaskResource<Memory::RegionableVirtualAddressSpace>(Memory::RegionableVirtualAddressSpace(Memory::MemoryManager::instance().get_main_address_space())), SharableTaskResource<FileDescriptorTable>(FileDescriptorTable()), SharableTaskResource<EventDescriptorTable>(EventDescriptorTable()), taskPriority);
    }

    void TaskManager::switch_to_task(Task* task)
    {
        // Acquire spinlock if we do not already have it.
        if (!is_spinlock_acquired()) { m_spinlock.acquire(); }

        if (task->m_state == Task::TaskState::NOT_STARTED)
        {
            // If the task has not already been started then we need to fake an iret stack.
            task->m_state = Task::TaskState::READY;
            Memory::MemoryManager::instance().switch_to_address_space(task->get_tlTable());
            CPU::update_kernel_stack_pointer(task->get_kstack());
            auto entryPoint = task->get_entry_point();
            auto kstack = task->get_kstack();
            m_spinlock.release();
            start_kernel_task_iretq(entryPoint, kstack);
        }
        else
        {
            // Update kernel stack pointer but add the size of the iretq stack as this will be popped.
            CPU::update_kernel_stack_pointer(reinterpret_cast<void*>(reinterpret_cast<uint64_t>(task->get_kstack()) + SIZE_OF_IRETQ_STACK));
            auto tlTable = (uint64_t*)((*(task->m_tlTable)).get_physical_address().get());
            m_spinlock.release();
            return_to_task(tlTable);
        }
    }

    void TaskManager::add_task(Task task)
    {
        m_spinlock.acquire();
        m_scheduler->add_task(task);
        m_spinlock.release();
    }

    void TaskManager::pit_tick()
    {
        send_pit_eoi();
        auto currentTime = CPU::PIT::time_since_boot();

        // Check for sleeping tasks and unblock any that have slept for the correct amount of time.
        while (TaskManager::instance().m_sleepingTasks.size() > 0)
        {
            auto task = TaskManager::instance().m_sleepingTasks.top();
            if (currentTime >= task.wakeTime)
            {
                TaskManager::instance().m_sleepingTasks.pop();
                TaskManager::instance().unblock_task(task.task);
            }
            else
                break;
        }
        // If the scheduler is already changing task, let the scheduler refresh.
        if (TaskManager::instance().is_scheduler_changing_task())
            return;
        TaskManager::instance().refresh();
    }

    void TaskManager::sleep_for(uint64_t duration)
    {
        auto currentTime = CPU::PIT::time_since_boot();
        TaskManager::instance().sleep_until(currentTime + duration);
    }

    void TaskManager::sleep_until(uint64_t time)
    {
        m_spinlock.acquire();
        auto currentTime = CPU::PIT::time_since_boot();
        if (currentTime >= time)
        {
            m_spinlock.release();
            return;
        }

        SleepingTask sleepingTask;
        sleepingTask.task = get_current_task();
        sleepingTask.wakeTime = time;
        // Add sleeping task to list of sleeping tasks that is checked on every timer interrupt.
        m_sleepingTasks.push(sleepingTask);
        block_task();
    }

    void TaskManager::refresh()
    {
        if (!TaskManager::instance().is_spinlock_acquired()) { TaskManager::instance().m_spinlock.acquire(); }

        TaskManager::instance().m_schedulerChangingTask = true;
        TaskManager::instance().m_scheduler->move_to_next_task(TaskManager::instance().m_spinlock);
        TaskManager::instance().m_schedulerChangingTask = false;
        auto currentTask = TaskManager::instance().m_scheduler->get_current_task();
        TaskManager::instance().switch_to_task(TaskManager::instance().m_scheduler->get_task_from_tid(currentTask));
    }
