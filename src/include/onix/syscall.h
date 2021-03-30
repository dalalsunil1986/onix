#ifndef ONIX_SYSCALL_H
#define ONIX_SYSCALL_H

#include <onix/types.h>

enum SYSCALL_NR
{
    SYS_NR_TEST,
    SYS_NR_GETPID
};

extern u32 syscall0(u32 nr);
extern u32 syscall1(u32 nr, u32 arg);
extern u32 syscall2(u32 nr, u32 arg1, u32 arg2);
extern u32 syscall3(u32 nr, u32 arg1, u32 arg2, u32 arg3);

u32 sys_test();
u32 sys_getpid();

#endif