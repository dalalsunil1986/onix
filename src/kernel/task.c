/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#include <onix/debug.h>
#include <onix/console.h>

void idle_task()
{
    while (1)
    {
        blink_char('I', 79, 0);
    }
}

void init_task()
{
    while (1)
    {
        blink_char('K', 77, 0);
    }
}