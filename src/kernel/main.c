#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>

Descriptor gdt[GDT_SIZE];
Pointer gdt_ptr;

void init_gdt()
{
    printk("Initializing GDT descriptor...\n");
    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(Pointer) * GDT_SIZE - 1;
}

void init_kernel()
{
    init_gdt();
}

int main()
{
     printk("Onix, is running....\n");
}