#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>

u16 get_cursor()
{
    u16 pos = 0;
    io_outb(CRTC_ADDR_REG, CURSOR_L);
    pos |= io_inb(CRTC_DATA_REG);
    io_outb(CRTC_ADDR_REG, CURSOR_H);
    pos |= ((u16)io_inb(CRTC_DATA_REG)) << 8;
    return pos;
}

u16 get_x()
{
    u16 pos = get_cursor();
    return pos % VGA_WIDTH;
}
u16 get_y()
{
    u16 pos = get_cursor();
    return pos / VGA_WIDTH;
}

void set_cursor(u16 x, u16 y)
{
    u16 pos = y * VGA_WIDTH + x;
    io_outb(0x3D4, CURSOR_L);
    io_outb(0x3D5, (u8)(pos & 0xFF));
    io_outb(0x3D4, CURSOR_H);
    io_outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void put_char(char ch)
{
    // u16 cpos = get_cursor();
    // char *current = (char *)V_MEM_BASE + cpos * 2;
    // *current = ch;
    // cpos += 1;
    // set_cursor()
}