#ifndef ONIX_SYSCALL_H
#define ONIX_SYSCALL_H

#include <onix/types.h>

extern u32 syscall0(u32 nr);
extern u32 syscall1(u32 nr);
extern u32 syscall2(u32 nr);
extern u32 syscall3(u32 nr);

void test_syscall();

#endif