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

int get_x(int pos)
{
    return pos % VGA_WIDTH;
}

int get_y(int pos)
{
    return pos / VGA_WIDTH;
}

int get_pos(int x, int y)
{
    return y * VGA_WIDTH + x;
}

void put_char(char ch)
{
    // BMB;
    int cpos = get_cursor();
    char *current = (char *)V_MEM_BASE + cpos * 2;

    u16 x = get_x(cpos);
    u16 y = get_y(cpos);

    switch (ch)
    {
    case '\b':
        x = x >= 1 ? x - 1 : 0;
        *current = ' ';
        cpos = get_pos(x, y);
    case '\n':
        cpos = get_pos(x, y + 1);
        break;
    default:
        *current = ch;
        cpos++;
        break;
    }
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
