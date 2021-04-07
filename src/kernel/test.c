#include <onix/syscall.h>
#include <onix/stdio.h>

#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

void test_processa()
{
    // clear();
    while (true)
    {
        u32 id = sys_getpid();
        printf("Hello test process %d\n", id);
        sys_sleep(200);
        break;
    }
}

void test_task()
{
    printf("Hello test task, exit...\n");
    sys_sleep(1000);
}