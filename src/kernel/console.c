/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/io.h>
#include <onix/console.h>

u16 get_cursor()
{
    u16 value = 0;
    outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
    value |= inb(CRT_DATA_REG) << 8;
    outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
    value |= inb(CRT_DATA_REG);
    return value;
}

void set_cursor(u16 cursor)
{
    u16 value = 0;
    outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
    outb(CRT_DATA_REG, cursor >> 8);
    outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
    outb(CRT_DATA_REG, cursor & 0xff);
}

void clear()
{
    char_t *ptr = (char_t *)VGA_MEM_BASE;
    u32 count = VGA_TEXT_SIZE;
    while (count--)
    {
        ptr->text = ' ';
        ptr->style = VGA_DEFAULT_STYLE;
        ptr++;
    }
    set_cursor(0);
}

static inline int get_x(int cursor)
{
    return cursor % VGA_WIDTH;
}

static inline int get_y(int cursor)
{
    return cursor / VGA_WIDTH;
}

static inline int get_pos(int x, int y)
{
    return y * VGA_WIDTH + x;
}

void put_char(char ch)
{
    u32 cursor = get_cursor();
    char_t *ptr = (char_t *)VGA_MEM_BASE;

    u32 x = get_x(cursor);
    u32 y = get_y(cursor);

    switch (ch)
    {
    case '\b':
        x = x >= 1 ? x - 1 : 0;
        cursor--;
        ptr[cursor].text = ' ';
        cursor = get_pos(x, y);
        break;
    case '\n':
    case '\r':
        cursor = get_pos(0, y + 1);
        break;
    case '\t':
        break;
    case '\x1b': // esc
        break;
    default:
        ptr[cursor].text = ch;
        cursor++;
        break;
    }
    set_cursor(cursor);
}