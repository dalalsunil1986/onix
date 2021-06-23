/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#include <onix/debug.h>
#include <onix/console.h>

void idle_task()
{
    clear();
    u32 counter = 0;
    while (true)
    {
        blink_char('I', 79, 0);
        // DEBUGK("idle task...\n");
    }
}

void init_task()
{
    clear();
    u32 counter = 0;
    while (true)
    {
        blink_char('K', 77, 0);
        // DEBUGK("init task...\n");
    }
}