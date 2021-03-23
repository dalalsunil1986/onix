#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/string.h>

Descriptor gdt[GDT_SIZE];
Pointer gdt_ptr;

void init_gdt()
{
    printk("Initializing GDT descriptor...\n");
    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(Pointer) * GDT_SIZE - 1;
    load_gdt(&gdt_ptr);
}

void init_descriptor(Descriptor *desc, u32 base, u32 limit)
{
    desc->base_low = base & 0xffffff;
    desc->base_high = (base >> 24) & 0xff;
    desc->limit_low = limit & 0xffff;
    desc->limit_high = (limit >> 16) & 0xf;
}