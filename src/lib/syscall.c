#include <onix/syscall.h>

u32 sys_test()
{
    return syscall0(SYS_NR_TEST);
}

u32 sys_getpid()
{
    return syscall0(SYS_NR_GETPID);
}