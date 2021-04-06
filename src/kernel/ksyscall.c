#include <onix/kernel/ksyscall.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/task.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/clock.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/harddisk.h>
#include <onix/syscall.h>
#include <onix/string.h>
#include <fs/path.h>
#include <fs/onix/fssyscall.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern void syscall_handler();
SyscallHandler syscall_table[SYSCALL_SIZE];

void __sys_default()
{
    printk("default syscall....\n");
}

void __sys_test()
{
    DEBUGP("syscall test called\n");
}

u32 __sys_getpid()
{
    Task *task = running_task();
    return task->pid;
}

void __sys_clear()
{
    __clear();
}

u32 __sys_write(char *str)
{
    printk("%s", str);
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
    u32 count = size < MAX_PATH_LEN ? size : MAX_PATH_LEN;
    memcpy(buf, task->cwd, count);
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
    abspath(path, task->cwd);
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
    syscall_table[SYS_NR_GETPID] = __sys_getpid;
    syscall_table[SYS_NR_WRITE] = __sys_write;
    syscall_table[SYS_NR_CLEAR] = __sys_clear;
    syscall_table[SYS_NR_SLEEP] = __sys_sleep;
    syscall_table[SYS_NR_MALLOC] = __sys_malloc;
    syscall_table[SYS_NR_FREE] = __sys_free;
}