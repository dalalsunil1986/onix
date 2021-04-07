#include <onix/stdarg.h>
#include <onix/syscall.h>
#include <onix/kernel/debug.h>

extern bool kernel_running;

int printf(const char *fmt, ...)
{

    char buf[1024] = {0};
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);

    if (!kernel_running)
    {
        return printk(buf);
    }
    else
    {
        sys_write(buf);
    }
    return i;
}

void clear()
{
    sys_clear();
}