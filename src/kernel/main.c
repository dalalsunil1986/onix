/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/debug.h>
#include <onix/console.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/global.h>
#include <onix/memory.h>
#include <onix/interrupt.h>
#include <onix/thread.h>

int main()
{
    init_gdt();
    init_memory();
    init_thread();
    init_interrupt();
    return 0;
}
