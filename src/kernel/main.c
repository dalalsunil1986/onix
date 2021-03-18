#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

void init_kernel()
{
    init_gdt();
}

int main()
{
     printk("Onix, is running....\n");
}