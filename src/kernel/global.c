#include <onix/kernel/global.h>

Descriptor gdt[GDT_SIZE];
Pointer gdt_ptr;

void init_gdt()
{
    printk("Initializing GDT descriptor...\n");
    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(Pointer) * GDT_SIZE - 1;
}