#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/interrupt.h>

void init_harddisk()
{
    printk("Initializing harddisk...\n");
    u16 harddisk_count = *((u8 *)(0x475));

    DEBUGK("harddisk count %d\n", harddisk_count);

    handler_table[ICW2_INT_VECTOR_IRQ0 + IRQ_HARD_DISK_1] = harddisk_handler;
}

void harddisk_handler(int vector)
{
    DEBUGK("Harddisk interrupt occured!!!");
}
