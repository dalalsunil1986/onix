#include <onix/kernel/printk.h>
#include <onix/stdarg.h>

void debugk(char *file, int line, const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    printk("[%s] [%d] %s", file, line, buf);
}