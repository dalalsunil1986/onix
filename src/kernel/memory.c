/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/memory.h>
#include <onix/debug.h>

u32 memory_base;
u32 memory_size;
u32 total_pages;
u32 free_pages;

void init_ards(ards_t *ards, u32 count)
{
    memory_base = 0;
    memory_size = 0;
    for (size_t i = 0; i < count; i++, ards++)
    {
        if (ards->type != 1)
            continue;
        if (ards->size > memory_size)
        {
            memory_size = ards->size;
            memory_base = ards->base;
        }
    }

    u32 offset = memory_base & 0xfff;

    if (offset != 0)
    {
        memory_base = (memory_base / PAGE_SIZE + 1) * PAGE_SIZE;
        memory_size -= offset;
    }

    total_pages = memory_size / PAGE_SIZE;
    free_pages = total_pages;

    INFOK("System Memory Base 0x%08X Size 0x%08X\n", memory_base, memory_size);
    INFOK("System Memory Total 0x%X\n", total_pages);
}

void init_memory()
{
}

page_table_t get_pde()
{
    asm volatile("movl %cr3, %eax\n");
}

void set_pde(page_table_t pde)
{
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}