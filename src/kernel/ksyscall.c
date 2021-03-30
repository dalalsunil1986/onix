#include <onix/kernel/ksyscall.h>
#include <onix/kernel/interrupt.h>

extern void syscall_handler();
SyscallHandler syscall_table[SYSCALL_SIZE];

void init_syscall()
{
    InterruptHandler handler = syscall_handler;
    InterruptGate *gate = &idt[INT_VECTOR_SYSCALL];
    gate->offset0 = (u32)handler & 0xffff;
    gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
    gate->DPL = PL3;
}