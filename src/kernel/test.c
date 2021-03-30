#include <onix/syscall.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

void test_processa()
{
    u32 counter = 0;
    while (true)
    {
        counter++;
        char ch = ' ';
        if ((counter % 2) != 0)
        {
            ch = 'T';
        }
        show_char(ch, 73, 0);
        test_syscall();

        // DEBUGP("test process a .....\n");// 此时应该没有 io 权限无法修改 光标会报 gp 异常
        // sleep(100);
    }
}
