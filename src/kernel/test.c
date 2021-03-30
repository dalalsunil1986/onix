#include <onix/syscall.h>
#include <onix/stdio.h>

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
        u32 id = getpid();
        printf("Hello test process %d\n", id);
    }
}
