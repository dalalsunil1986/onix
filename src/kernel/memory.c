/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/memory.h>
#include <onix/debug.h>

u32 memory_base;
u32 memory_size;

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
    DEBUGK("System Memory Base 0x%08X Size 0x%08X\n", memory_base, memory_size);
}