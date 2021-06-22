#include <onix/global.h>
#include <onix/assert.h>
#include <onix/string.h>
#include <onix/debug.h>

descriptor_t gdt[GDT_SIZE];
pointer_t gdt_ptr;

void init_gdt()
{
    BOCHS_MAGIC_BREAKPOINT;

    DEBUGK("init GDT...\n");

    asm volatile("sgdt gdt_ptr\n");

    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);

    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(pointer_t) * GDT_SIZE - 1;

    asm volatile("lgdt gdt_ptr\n");
}
