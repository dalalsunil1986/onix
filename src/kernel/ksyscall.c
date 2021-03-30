#include <onix/kernel/ksyscall.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/clock.h>
#include <onix/syscall.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern void syscall_handler();
SyscallHandler syscall_table[SYSCALL_SIZE];

void __sys_test()
{
    DEBUGP("syscall test called\n");
}

u32 __sys_getpid()
{
    Task *task = running_task();
    return task->id;
}

void init_syscall()
{
    InterruptHandler handler = syscall_handler;
    InterruptGate *gate = &idt[INT_VECTOR_SYSCALL];
    gate->offset0 = (u32)handler & 0xffff;
    gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
    gate->DPL = PL3;

    syscall_table[SYS_NR_TEST] = __sys_test;
    syscall_table[SYS_NR_GETPID] = __sys_getpid;
}