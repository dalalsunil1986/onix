#include <onix/kernel/global.h>
#include <onix/kernel/process.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/memory.h>

static TSS tss;

void init_tss()
{
    printk("Initilaizing TSS...\n\0");
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = *(u32 *)&SELECTOR_KERNEL_DATA;
    tss.iobase = sizeof(tss);

    Descriptor *tss_desc = gdt + SELECT_KERNEL_TSS_INDEX;
    init_descriptor(tss_desc, get_paddr(&tss), sizeof(tss) - 1);
    tss_desc->granularity = 0;
    tss_desc->big = 0;
    tss_desc->long_mode = 0;
    tss_desc->present = 1;
    tss_desc->DPL = PL0;
    tss_desc->code = 1;
    tss_desc->conform_expand = 0;
    tss_desc->read_write = 0; // busy
    tss_desc->accessed = 1;

    load_tss(&SELECTOR_KERNEL_TSS);
}

void init_process()
{
    init_tss();
}