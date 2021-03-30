#ifndef ONIX_SYSCALL_H
#define ONIX_SYSCALL_H

#include <onix/types.h>

enum SYSCALL_NR
{
    SYS_NR_GETPID,
    SYS_NR_WRITE,
    SYS_NR_SLEEP,
    SYS_NR_TEST
};

extern u32 syscall0(u32 nr);
extern u32 syscall1(u32 nr, u32 arg);
extern u32 syscall2(u32 nr, u32 arg1, u32 arg2);
extern u32 syscall3(u32 nr, u32 arg1, u32 arg2, u32 arg3);

u32 sys_test();
u32 sys_getpid();
u32 sys_write(char *str);

void sys_sleep(u32 milliseconds);

#endif