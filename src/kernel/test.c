#include <onix/syscall.h>
#include <onix/stdio.h>
#include <onix/assert.h>

#include <onix/syscall.h>
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
    clear();
    DEBUGP("Task 0x%X process %d fork....\n", running_task(), sys_getpid());
    u32 id = sys_fork();
    const int size = 16;
    char buf[size];
    while (true)
    {
        printf("[steven@onix]#");
        sys_read(onix_stdin, buf, size);
        buf[15] = 0;
        DEBUGP("Process %d read %s\n", sys_getpid(), buf);
        break;
    }
    DEBUGP("test process %d exit....\n", sys_getpid());
}
