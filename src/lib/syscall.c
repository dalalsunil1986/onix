#include <onix/syscall.h>
#include <fs/file.h>
#ifdef ONIX_KERNEL_DEBUG
#include <onix/kernel/ksyscall.h>
#endif

u32 sys_test()
{
    return syscall0(SYS_NR_TEST);
}

void sys_exit(u32 code)
{
    syscall1(SYS_NR_EXIT, code);
    return;
}

void sys_wait(int32 *status)
{
    syscall1(SYS_NR_WAIT, status);
}

u32 sys_fork()
{
    return syscall0(SYS_NR_FORK);
}

u32 sys_getpid()
{
    return syscall0(SYS_NR_GETPID);
}

void sys_ps()
{
    return syscall0(SYS_NR_PS);
}

void sys_sleep(u32 milliseconds)
{
    syscall1(SYS_NR_SLEEP, milliseconds);
}

u32 sys_read(fd_t fd, void *buf, u32 count)
{
    return syscall3(SYS_NR_READ, fd, buf, count);
}

u32 sys_write(fd_t fd, void *buf, u32 count)
{
    return syscall3(SYS_NR_WRITE, fd, buf, count);
}

void sys_clear()
{
    syscall0(SYS_NR_CLEAR);
}

void sys_putchar(char ch)
{
    return syscall1(SYS_NR_PUTCHAR, ch);
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

int32 sys_open(const char *pathname, FileFlag flags);

int32 sys_unlink(const char *pathname)
{
    return syscall1(SYS_NR_ULINK, pathname);
}

int32 sys_mkdir(const char *pathname)
{
    return syscall1(SYS_NR_MKDIR, pathname);
}

int32 sys_rmdir(const char *pathname)
{
    return syscall1(SYS_NR_RMDIR, pathname);
}

int32 sys_opendir(const char *pathname)
{
    return syscall1(SYS_NR_OPENDIR, pathname);
}

int32 sys_closedir(Dir *dir)
{
    return syscall1(SYS_NR_CLOSEDIR, dir);
}

DirEntry *sys_readdir(Dir *dir)
{
    return syscall1(SYS_NR_READDIR, dir);
}

void sys_rewinddir(Dir *dir)
{
    return syscall1(SYS_NR_REWINDDIR, dir);
}