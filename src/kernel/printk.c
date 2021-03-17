#include <onix/stdarg.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>

static char buf[1024];

int get_cursor()
{
    int pos = 0;
    io_outb(CRTC_ADDR_REG, CURSOR_L);
    pos |= io_inb(CRTC_DATA_REG);
    io_outb(CRTC_ADDR_REG, CURSOR_H);
    pos |= ((u16)io_inb(CRTC_DATA_REG)) << 8;
    return pos;
}

void set_cursor(int pos)
{
    io_outb(CRTC_ADDR_REG, CURSOR_L);
    io_outb(CRTC_DATA_REG, (u8)(pos & 0xFF));
    io_outb(CRTC_ADDR_REG, CURSOR_H);
    io_outb(CRTC_DATA_REG, (u8)((pos >> 8) & 0xFF));
}

void put_char(char ch)
{
    // BMB;
    auto cpos = get_cursor();

    char *current = (char *)V_MEM_BASE + cpos * 2;

    // BOCHS_MAGIC_BREAKPOINT;

    *current = ch;

    cpos++;
    set_cursor(cpos);
}

int printk(const char *fmt, ...)
{
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
    return i;
}
