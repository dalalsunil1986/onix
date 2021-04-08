#include <onix/syscall.h>
#include <onix/stdio.h>
#include <onix/assert.h>

#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGF
#else
#define DEBUGP(fmt, args...)
#endif

void test_processa()
{
    // clear();
    DEBUGP("Task 0x%X process %d fork....\n", running_task(), sys_getpid());
    // u32 id = sys_fork();
    while (true)
    {
        DEBUGP("Hello test process %d\n", sys_getpid());
        // break;
    }
    DEBUGP("test process %d exit....\n", sys_getpid());
}

void test_task()
{
    DEBUGP("Hello test task, exit...\n");
    sys_sleep(1000);
}