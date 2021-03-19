#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>

u32 total_memory_bytes;
u32 ards_count;
ARDS ards_table[ARDS_SIZE];

void init_memory()
{
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    printk("Total Memory Ards count %d\n", ards_count);

    for (size_t i = 0; i < ards_count; i++)
    {
        ARDS ards = ards_table[i];
        printk("ARDS %d Addr 0x%08X%08X Size 0x%08X%08X Type %d \n", i,
               ards.addr1, ards.addr0,
               ards.size1, ards.size0,
               ards.type);
    }
}