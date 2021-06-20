/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/console.h>
#include <onix/printk.h>
#include <onix/assert.h>

int main()
{
    char *video = 0xb8000;
    *video = 'A';

    u16 cursor = get_cursor();
    cursor = 1;
    set_cursor(cursor);
    cursor = get_cursor();
    clear();

    u32 count = 1;
    while (count++)
    {
        printk("hello, onix %d\n", count);
        break;
    }
    assert(true);
    assert(false);

    return 0;
}
