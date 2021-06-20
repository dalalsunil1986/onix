/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/console.h>
#include <onix/printk.h>

int main()
{
    char *video = 0xb8000;
    *video = 'A';

    u16 cursor = get_cursor();
    cursor = 1;
    set_cursor(cursor);
    cursor = get_cursor();
    clear();

    char *message = "hello world!!!\n\0";
    printk(message);

    return 0;
}
