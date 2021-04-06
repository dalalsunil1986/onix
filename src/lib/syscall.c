#include <onix/syscall.h>
#include <fs/file.h>
#ifdef ONIX_KERNEL_DEBUG
#include <onix/kernel/ksyscall.h>
#endif

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

char *sys_getcwd(char *buf, u32 size)
{
#ifndef ONIX_KERNEL_DEBUG
    return syscall2(SYS_NR_CWD, buf, size);
#else
    return __sys_getcwd(buf, size);
#endif
}

int32 sys_stat(const char *pathname, Stat *stat)
{
#ifndef ONIX_KERNEL_DEBUG
    return syscall2(SYS_NR_STAT, pathname, stat);
#else
    return __sys_stat(pathname, stat);
#endif
}
