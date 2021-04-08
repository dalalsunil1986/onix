#include <onix/assert.h>
#include <onix/stdio.h>
#include <onix/kernel/io.h>
#include <onix/kernel/task.h>
#include <onix/kernel/printk.h>
#include <onix/stdarg.h>

int assert_print(char *fmt, ...)
{
    char buf[1024] = {0};
    va_list args;
    int i;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);

    u32 stack = running_task();
    if (stack < KERNEL_ADDR_MASK)
        printf(buf);
    else
        printk(buf);
    return i;
}

void spin(char *name)
{
    assert_print("\nspinning in %s ...\n\0", name);
    while (true)
        ;
}

void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    assert_print(
        "%c \n--> assert(%s) failed!!!\n"
        "--> file: %s \n"
        "--> base_file: %s, \n"
        "--> line %d\n\0",
        MAG_CH_ASSERT,
        exp, file, base_file, line);

    spin("assertion_failure()");

    /* should never arrive here */
    __asm__ __volatile__("ud2");
}

void panic(const char *format, ...)
{
    int i;
    char buf[256];

    /* 4 is the size of fmt in the stack */
    va_list arg = (va_list)((char *)&format + 4);

    i = vsprintf(buf, format, arg);

    assert_print("%c !!panic!! %s \n", MAG_CH_PANIC, buf);

    spin("panic");

    /* should never arrive here */
    __asm__ __volatile__("ud2");
}
