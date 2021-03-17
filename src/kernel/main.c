#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

int main()
{
    // BOCHS_MAGIC_BREAKPOINT;
    clear();
    int n = 30;
    while (n--)
    {
        printk("Onix.... %d \n", n);

    }

    while (1)
    {
    }
    return 0;
}