#ifndef ONIX_SYSCALL_H
#define ONIX_SYSCALL_H

#include <onix/types.h>
#include <fs/file.h>

enum SYSCALL_NR
{
    SYS_NR_TEST,
    SYS_NR_EXIT,
    SYS_NR_FORK,
    SYS_NR_READ,
    SYS_NR_WRITE,
    SYS_NR_PUTCHAR,
    SYS_NR_OPEN,
    SYS_NR_CLOSE,
    SYS_NR_WAITPID,
    SYS_NR_CREATE,
    SYS_NR_LINK,
    SYS_NR_ULINK,
    SYS_NR_OPENDIR,
    SYS_NR_CLOSEDIR,
    SYS_NR_READDIR,
    SYS_NR_REWINDDIR,
    SYS_NR_EXEC,
    SYS_NR_CHDIR,
    SYS_NR_TIME,
    SYS_NR_MKNOD,
    SYS_NR_CHMOD,
    SYS_NR_CHOWN,
    SYS_NR_BREAK,
    SYS_NR_STAT,
    SYS_NR_LSEEK,
    SYS_NR_GETPID,
    SYS_NR_PS,
    SYS_NR_MOUNT,
    SYS_NR_UMOUNT,
    SYS_NR_SETUID,
    SYS_NR_GETUID,
    SYS_NR_STIME,
    SYS_NR_CLEAR,
    SYS_NR_SLEEP,
    SYS_NR_PAUSE,
    SYS_NR_RENAME,
    SYS_NR_MKDIR,
    SYS_NR_RMDIR,
    SYS_NR_MALLOC,
    SYS_NR_FREE,
    SYS_NR_GETCWD,
};

extern u32 syscall0(u32 nr);
extern u32 syscall1(u32 nr, u32 arg);
extern u32 syscall2(u32 nr, u32 arg1, u32 arg2);
extern u32 syscall3(u32 nr, u32 arg1, u32 arg2, u32 arg3);

u32 sys_test();

void sys_exit(u32 code);
void sys_sleep(u32 milliseconds);
u32 sys_fork();
u32 sys_getpid();
void sys_ps();

void sys_clear();
void sys_putchar(char ch);

u32 sys_malloc(size_t size);
void sys_free(void *ptr);

char *sys_getcwd(char *buf, u32 size);
int32 sys_chdir(const char *path);
int32 sys_stat(const char *pathname, Stat *stat);

u32 sys_read(fd_t fd, void *buf, u32 count);
u32 sys_write(fd_t fd, void *buf, u32 count);
int32 sys_unlink(const char *pathname);

int32 sys_mkdir(const char *pathname);
int32 sys_rmdir(const char *pathname);
int32 sys_opendir(const char *pathname);
int32 sys_closedir(Dir *dir);
DirEntry *sys_readdir(Dir *dir);
void sys_rewinddir(Dir *dir);

#endif