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

u32 sys_read(fd_t fd, void *buf, u32 count)
{
    return syscall3(SYS_NR_READ, fd, buf, count);
}

u32 sys_write(char *str)
{
    return syscall1(SYS_NR_WRITE, str);
}

void sys_putchar(char ch)
{
    return syscall1(SYS_NR_PUTCHAR, ch);
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
    return syscall2(SYS_NR_GETCWD, buf, size);
#else
    return __sys_getcwd(buf, size);
#endif
}

int32 sys_chdir(const char *path)
{
    return syscall1(SYS_NR_CHDIR, path);
}

int32 sys_stat(const char *pathname, Stat *stat)
{
#ifndef ONIX_KERNEL_DEBUG
    return syscall2(SYS_NR_STAT, pathname, stat);
#else
    return __sys_stat(pathname, stat);
#endif
}

void sys_exit(u32 code)
{
    syscall1(SYS_NR_EXIT, code);
    return;
}

u32 sys_fork()
{
    return syscall0(SYS_NR_FORK);
}