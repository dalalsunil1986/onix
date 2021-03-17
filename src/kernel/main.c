#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

Descriptor gdt[GDT_SIZE];
Pointer gdt_ptr;

void init_gdt()
{
    printk("Initializing GDT descriptor...\n");
    memcpy(&gdt, gdt_ptr.base, gdt_ptr.limit + 1);
    gdt_ptr.base = &gdt;
    gdt_ptr.limit = sizeof(Pointer) * GDT_SIZE - 1;
}

void init_kernel()
{
    init_gdt();
}

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