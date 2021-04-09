#include <onix/kernel/ksyscall.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/task.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/clock.h>
#include <onix/assert.h>
#include <onix/kernel/harddisk.h>
#include <onix/syscall.h>
#include <onix/string.h>
#include <fs/path.h>
#include <fs/onix/fssyscall.h>
#include <onix/kernel/ioqueue.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern IOQueue key_ioq;
extern void syscall_handler();
SyscallHandler syscall_table[SYSCALL_SIZE];

char pathbuf[MAX_PATH_LEN];

void __sys_default()
{
    printk("!!!default syscall....\n");
}

void __sys_test()
{
    print_irq_mask();
    DEBUGP("syscall test called\n");
}

void __sys_exit(u32 code)
{
    // DEBUGP("syscall exit called\n");
    Task *task = running_task();
    task->exit_code = code;
    task_exit(task);
}

u32 __sys_fork()
{
    return task_fork();
}

u32 __sys_getpid()
{
    Task *task = running_task();
    return task->pid;
}

extern Queue tasks_queue;

void __sys_ps()
{
    Node *node = tasks_queue.tail.prev;
    while (node != &tasks_queue.head)
    {
        Task *task = element_entry(Task, all_node, node);
        printk("%d %08X %s\n", task->tid, task, task->name);
        node = node->prev;
    }
}

void __sys_sleep(u32 milliseconds)
{
    clock_sleep(milliseconds);
}

u32 __sys_malloc(size_t size)
{
    return arena_malloc(size);
}

void __sys_free(void *ptr)
{
    arena_free(ptr);
}

char *__sys_getcwd(char *buf, u32 size)
{
    assert(buf != NULL);

    Task *task = running_task();
    u32 length = strlen(task->cwd);
    u32 count = size < length ? size : length;
    memcpy(buf, task->cwd, count);
    buf[count] = 0;
    return buf;
}

int32 __sys_chdir(const char *path)
{
    if (!exists(path))
    {
        printk("%s not exists...\n", path);
        return -1;
    }
    u32 size = strlen(path);
    assert(size <= MAX_PATH_LEN);
    Task *task = running_task();
    memset(pathbuf, 0, MAX_PATH_LEN);
    abspath(path, pathbuf);
    memcpy(task->cwd, pathbuf, MAX_PATH_LEN);
    return 0;
}

int32 __sys_stat(const char *pathname, Stat *stat)
{
    Partition *part = get_path_part(pathname);
    switch (part->fs)
    {
    case FS_ONIX:
        return onix_sys_stat(pathname, stat);
    default:
        break;
    }
    return -1;
}

int32 __sys_read(fd_t fd, void *buf, u32 count)
{
    assert(buf != NULL);
    int32 ret = -1;
    if (fd < 0 || fd == onix_stdout || fd == onix_stderr)
    {
        printk("sys_read: fd error\n");
        return ret;
    }
    if (fd == onix_stdin)
    {
        char *ptr = buf;
        u32 idx = 0;
        while (idx < count)
        {
            ptr = buf + idx;
            *ptr = ioqueue_get(&key_ioq);
            idx++;
        }
        return idx;
    }
    ret = onix_sys_read(fd, buf, count);
    return ret;
}

int32 __sys_write(fd_t fd, void *buf, u32 count)
{
    if (fd == onix_stdout)
    {
        return printk(buf);
    }
}

fd_t __sys_open(const char *pathname, FileFlag flags);
fd_t __sys_close(fd_t fd);

int32 __sys_lseek(fd_t fd, int32 offset, Whence whence);

int32 __sys_unlink(const char *pathname)
{
    return onix_sys_unlink(pathname);
}

int32 __sys_mkdir(const char *pathname)
{
    return onix_sys_mkdir(pathname);
}

Dir *__sys_opendir(const char *pathname)
{
    return onix_sys_opendir(pathname);
}

int32 __sys_closedir(Dir *dir)
{
    return onix_sys_closedir(dir);
}

int32 __sys_rmdir(const char *pathname)
{
    return onix_sys_rmdir(pathname);
}

DirEntry *__sys_readdir(Dir *dir)
{
    return onix_sys_readdir(dir);
}

void __sys_rewinddir(Dir *dir)
{
    return onix_sys_rewinddir(dir);
}

void __sys_putchar(char ch)
{
    put_char(ch);
}

void __sys_clear()
{
    __clear();
}

void init_syscall()
{
    InterruptHandler handler = syscall_handler;
    InterruptGate *gate = &idt[INT_VECTOR_SYSCALL];
    gate->offset0 = (u32)handler & 0xffff;
    gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
    gate->DPL = PL3;

    for (size_t i = 0; i < SYSCALL_SIZE; i++)
    {
        syscall_table[i] = __sys_default;
    }

    syscall_table[SYS_NR_TEST] = __sys_test;
    syscall_table[SYS_NR_EXIT] = __sys_exit;
    syscall_table[SYS_NR_FORK] = __sys_fork;
    syscall_table[SYS_NR_GETPID] = __sys_getpid;
    syscall_table[SYS_NR_PS] = __sys_ps;
    syscall_table[SYS_NR_SLEEP] = __sys_sleep;

    syscall_table[SYS_NR_MALLOC] = __sys_malloc;
    syscall_table[SYS_NR_FREE] = __sys_free;

    syscall_table[SYS_NR_PUTCHAR] = __sys_putchar;
    syscall_table[SYS_NR_CLEAR] = __sys_clear;

    syscall_table[SYS_NR_CHDIR] = __sys_chdir;
    syscall_table[SYS_NR_GETCWD] = __sys_getcwd;

    syscall_table[SYS_NR_STAT] = __sys_stat;
    syscall_table[SYS_NR_READ] = __sys_read;
    syscall_table[SYS_NR_WRITE] = __sys_write;

    syscall_table[SYS_NR_ULINK] = __sys_unlink;

    syscall_table[SYS_NR_OPENDIR] = __sys_opendir;
    syscall_table[SYS_NR_CLOSEDIR] = __sys_closedir;
    syscall_table[SYS_NR_READDIR] = __sys_readdir;
    syscall_table[SYS_NR_REWINDDIR] = __sys_rewinddir;
    syscall_table[SYS_NR_MKDIR] = __sys_mkdir;
    syscall_table[SYS_NR_RMDIR] = __sys_rmdir;
}