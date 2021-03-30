#include <onix/kernel/syscall.h>
#include <onix/kernel/interrupt.h>

void syscall_handler(int vector)
{
}

void init_syscall()
{
    InterruptGate *gate = &idt[INT_VECTOR_SYSCALL];
    gate->DPL = PL3;
    handler_table[INT_VECTOR_SYSCALL] = syscall_handler;
}