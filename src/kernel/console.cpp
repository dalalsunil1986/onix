#include <onix/console.h>
#include <onix/io.h>

u16 onix::console::get_cursor()
{
    u16 value = 0;
    onix::io::outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
    value |= onix::io::inb(CRT_DATA_REG) << 8;
    onix::io::outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
    value |= onix::io::inb(CRT_DATA_REG);
    return value;
}

void onix::console::set_cursor(u16 cursor)
{
    u16 value = 0;
    onix::io::outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
    onix::io::outb(CRT_DATA_REG, cursor >> 8);
    onix::io::outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
    onix::io::outb(CRT_DATA_REG, cursor & 0xff);
}

void onix::console::clear()
{
    console::text *ptr = (console::text *)VGA_MEM_BASE;
    u32 count = VGA_TEXT_SIZE;
    while (count--)
    {
        ptr->text = ' ';
        ptr->style = VGA_DEFAULT_STYLE;
        ptr++;
    }
    set_cursor(0);
}

void onix::console::putchar(char text)
{
    u32 cursor = get_cursor();
    console::text *ptr = (console::text *)VGA_MEM_BASE;
    ptr[cursor].text = text;
    cursor++;
    set_cursor(cursor);
}