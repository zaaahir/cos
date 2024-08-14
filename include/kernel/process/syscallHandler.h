#ifndef SYSCALLHANDLER_H
#define SYSCALLHANDLER_H
#include "types.h"
#include "process/syscall.h"
#include "fs/vfs.h"

namespace Processes
{
    extern uint64_t syscall_handler(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
}

#endif
