#include <onix/kernel/ksyscall.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern void syscall_handler();
SyscallHandler syscall_table[SYSCALL_SIZE];

void syscall_test()
{
    DEBUGP("syscall test called\n");
}

void init_syscall()
{
    InterruptHandler handler = syscall_handler;
    InterruptGate *gate = &idt[INT_VECTOR_SYSCALL];
    gate->offset0 = (u32)handler & 0xffff;
    gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
    gate->DPL = PL3;

    syscall_table[0] = syscall_test;
}