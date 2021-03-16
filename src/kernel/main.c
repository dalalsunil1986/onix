#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

int main()
{

    // BOCHS_MAGIC_BREAKPOINT;

    put_char('O');
    put_char('n');
    put_char('i');
    put_char('x');

    set_cursor(0, 0);

    while (1)
    {
    }
    return 0;
}