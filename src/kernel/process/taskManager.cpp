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
