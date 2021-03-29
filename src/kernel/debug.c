#include <onix/kernel/printk.h>

void debugk(char *file, int line, const char *fmt, ...)
{
    char buf[1024];
    va_list arg = (va_list)((char *)&fmt + 4);
    vsprintf(buf, fmt, arg);
    printk("[%s] [%d] %s", file, line, buf);
}