#include <onix/stdarg.h>
#include <onix/string.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>

static char buf[1024];

extern int vsprintf(char *buf, const char *fmt, va_list args);

int get_cursor()
{
    int pos = 0;
    outb(CRTC_ADDR_REG, CURSOR_L);
    pos |= inb(CRTC_DATA_REG);
    outb(CRTC_ADDR_REG, CURSOR_H);
    pos |= ((u16)inb(CRTC_DATA_REG)) << 8;
    return pos;
}

void set_cursor(int pos)
{
    outb(CRTC_ADDR_REG, CURSOR_L);
    outb(CRTC_DATA_REG, (u8)(pos & 0xFF));
    outb(CRTC_ADDR_REG, CURSOR_H);
    outb(CRTC_DATA_REG, (u8)((pos >> 8) & 0xFF));
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

void clear()
{
    int cpos = 0;
    char *current = (char *)V_MEM_BASE;
    while (cpos++ < VGA_HEIGHT * VGA_HEIGHT * 2)
    {
        *current = ' ';
        current += 2;
    }
    set_cursor(0);
}

void scroll()
{
    char *dest = (char *)V_MEM_BASE;
    char *src = dest + (VGA_WIDTH * 2);
    memcpy(dest, src, (VGA_HEIGHT - 1) * (VGA_WIDTH * 2));
}

void put_char(char ch)
{
    // BMB;
    int cpos = get_cursor();
    if (cpos >= (VGA_HEIGHT - 1) * VGA_WIDTH)
    {
        scroll();
        cpos -= VGA_WIDTH;
    }
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
        cpos = get_pos(0, y + 1);
        break;
    default:
        *current = ch;
        cpos++;
        break;
    }
    if (cpos > (VGA_HEIGHT - 1) * VGA_WIDTH)
    {
        scroll();
        cpos -= VGA_WIDTH;
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

void show_char(char ch, int x, int y)
{
    char *video = (char *)V_MEM_BASE + (get_pos(x, y) * 2);
    *video = ch;
}