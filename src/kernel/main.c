#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

int main()
{
    // BOCHS_MAGIC_BREAKPOINT;

    printk("Onix.... %d", 123);

    while (1)
    {
    }
    return 0;
}