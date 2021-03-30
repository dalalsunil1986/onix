#include <onix/syscall.h>

u32 sys_test()
{
    return syscall0(SYS_NR_TEST);
}

u32 sys_getpid()
{
    return syscall0(SYS_NR_GETPID);
}

void sys_clear()
{
    syscall0(SYS_NR_CLEAR);
}

u32 sys_write(char *str)
{
    return syscall1(SYS_NR_WRITE, str);
}

void sys_sleep(u32 milliseconds)
{
    syscall1(SYS_NR_SLEEP, milliseconds);
}

u32 sys_malloc(size_t size)
{
    return syscall1(SYS_NR_MALLOC, size);
}

void sys_free(void *ptr)
{
    syscall1(SYS_NR_FREE, ptr);
    return;
}