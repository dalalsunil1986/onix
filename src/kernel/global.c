#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/debug.h>
#include <onix/string.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Descriptor gdt[GDT_SIZE];
Pointer gdt_ptr;

void init_gdt()
{
    printk("Initializing GDT...\n");
    save_gdt(&gdt_ptr);

    DEBUGP("Gdt base 0x%X limit 0x%X 0x%X\n", gdt_ptr.base, gdt_ptr.limit, sizeof(Descriptor));
    assert(gdt_ptr.base == 0x6003);
    assert(gdt_ptr.limit == (sizeof(Descriptor) * 4 - 1));

    memcpy(&gdt, (void *)gdt_ptr.base, gdt_ptr.limit + 1);
    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(Pointer) * GDT_SIZE - 1;

    Descriptor *desc = gdt + SELECTOR_USER_CODE_INDEX;
    init_descriptor(desc, 0, 0XFFFFF);
    desc->segment = 1;
    desc->granularity = 1;
    desc->big = 1;
    desc->long_mode = 0;
    desc->present = 1;
    desc->DPL = PL3;
    desc->type = 0b1010;

    desc = gdt + SELECTOR_USER_DATA_INDEX;
    init_descriptor(desc, 0, 0XFFFFF);
    desc->segment = 1;
    desc->granularity = 1;
    desc->big = 1;
    desc->long_mode = 0;
    desc->present = 1;
    desc->DPL = PL3;
    desc->type = 0b0010;

    load_gdt(&gdt_ptr);
}

void init_descriptor(Descriptor *desc, u32 base, u32 limit)
{
    desc->base_low = base & 0xffffff;
    desc->base_high = (base >> 24) & 0xff;
    desc->limit_low = limit & 0xffff;
    desc->limit_high = (limit >> 16) & 0xf;
}