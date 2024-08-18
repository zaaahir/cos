#include "process/syscallHandler.h"
#include "process/taskManager.h"
#include "print.h"

// TODO: Validate these syscalls so that everything is checked, as the arguments are from ring 3 and
// a broken/malicious program could cause a kernel crash
// TODO: Reimplement vmmap/vmumap syscalls to allow for file mapping

uint64_t sleep_for_syscall(uint64_t duration)
{
    Task::TaskManager::instance().sleep_for(duration);
}

uint64_t vmmap_syscall(uint64_t* addr, uint64_t len, uint64_t hint, uint64_t flags)
{
    //Task::TaskManager::instance().syscall_mmap(addr, len, hint, flags);
}

uint64_t vmumap_syscall(uint64_t addr, uint64_t len)
{

}

uint64_t fopen_syscall(uint64_t filename, uint64_t flags)
{
    /*auto process = Processes::Scheduler::instance()->get_current_process();
    int i = 0;
    auto fd = process->openFileDescriptors.first();
    for (; !fd.is_end(); ++fd)
    {
        if (fd->fd != i) { break; }
        i++;
    }
    auto vnode = Filesystem::VirtualFilesystemManager::instance()->open_file((char*)filename, flags);
    if (!vnode) {return -1;}
    Filesystem::taskfd pfd;
    pfd.fd = i;
    pfd.node = vnode;
    pfd.offset = 0;
    process->openFileDescriptors.insert(pfd, fd);
    
    return i;*/
}

uint64_t fread_syscall(uint64_t fd, uint64_t buf, uint64_t count)
{
    /*auto process = Processes::Scheduler::instance()->get_current_process();
    auto cfd = Processes::ProcessManager::instance()->get_pfd(fd, process);
    if (cfd->fd != fd) { return -1; }
    return Filesystem::VirtualFilesystemManager::instance()->read(cfd->node, cfd->offset, count, (void*)buf);*/
}

uint64_t fclose_syscall(uint64_t fd)
{
    /*auto process = Processes::Scheduler::instance()->get_current_process();
    auto cfd = Processes::ProcessManager::instance()->get_pfd(fd, process);
    if (cfd->fd != fd) { return -1; }
    // We should close the file here, but we only have an initrd mounted at the moment
    process->openFileDescriptors.remove(cfd);
    return 0;*/
}

uint64_t eregister_syscall(uint64_t event, uint64_t param)
{
    /*auto process = Processes::Scheduler::instance()->get_current_process();
    return Events::EventDispatcher::register_event_listener(process, (char*)event, (void*)param);*/
}

uint64_t elisten_syscall(uint64_t ed)
{
    /*auto process = Processes::Scheduler::instance()->get_current_process();
    Events::EventDispatcher::block_event_listen(process, ed);
    return 1;*/
}
