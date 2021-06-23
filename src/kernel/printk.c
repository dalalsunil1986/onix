/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/stdarg.h>
#include <onix/vsprintf.h>
#include <onix/console.h>
#include <onix/interrupt.h>

static char buf[1024];

int printk(const char *fmt, ...)
{
    bool intr = set_interrupt(false); // TODO lock

    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);

    int n = i;
    while (n-- > 0)
    {
        put_char(buf[i - n - 1]);
    }

    set_interrupt(intr);

    return i;
}
