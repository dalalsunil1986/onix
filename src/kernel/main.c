#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/memory.h>

void __init_kernel()
{
    init_gdt();
    init_memory();
    init_interrupt();
}

int main()
{
    // clear();
    // printk("code selector 0x%X\n", *(short *)&SELECTOR_KERNEL_CODE);
    // printk("data selector 0x%X\n", *(short *)&SELECTOR_KERNEL_DATA);
    // printk("video selector 0x%X\n", *(short *)&SELECTOR_KERNEL_VIDEO);
    // printk("code descriptor segment %d\n", gdt[1].segment);
    printk("Onix, is running....\n");
    return 0;
}