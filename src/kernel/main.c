/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/debug.h>
#include <onix/console.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/global.h>
#include <onix/bitmap.h>

int main()
{
    clear();
    init_gdt();
    return 0;
}
