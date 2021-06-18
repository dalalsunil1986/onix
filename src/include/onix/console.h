#pragma once

#include <onix/types.h>

namespace onix
{
    namespace console
    {
        const static u16 CRT_ADDR_REG = 0x3D4;
        const static u16 CRT_DATA_REG = 0x3D5;

        const static u8 CRT_CURSOR_HIGH = 0x0E;
        const static u8 CRT_CURSOR_LOW = 0x0F;

        const static u8 VGA_WIDTH = 80;
        const static u8 VGA_HEIGHT = 25;

        const static u32 VGA_MEM_BASE = 0xB8000;
        const static u16 VGA_MEM_SIZE = 0x8000;

        const static u16 VGA_TEXT_SIZE = (VGA_MEM_SIZE / 2);
        const static u16 VGA_TEXT_HEIGHT = (VGA_TEXT_SIZE / VGA_WIDTH);

        const static u8 VGA_DEFAULT_STYLE = 0b00000111;

        struct text
        {
            char text;
            char style;
        };

        void clear();
        void putchar(char text);

        u16 get_cursor();
        void set_cursor(u16 cursor);
    }
}