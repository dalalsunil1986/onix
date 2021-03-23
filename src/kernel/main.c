#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/process.h>

void __init_kernel()
{
    // BMB;
    init_gdt();
    init_interrupt();
    init_memory();
    init_process();
}

int main()
{
    // clear();
    // printk("code selector 0x%X\n", *(short *)&SELECTOR_KERNEL_CODE);
    // printk("data selector 0x%X\n", *(short *)&SELECTOR_KERNEL_DATA);
    // printk("video selector 0x%X\n", *(short *)&SELECTOR_KERNEL_VIDEO);
    // printk("code descriptor segment %d\n", gdt[1].segment);
    u32 counter = 0;
    while (++counter)
    {
        show_char(counter % 10 + 0x30, 77, 0);
    }
    return 0;
}