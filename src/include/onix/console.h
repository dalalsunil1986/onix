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

        u16 get_cursor();
        void set_cursor(u16 cursor);
    }
}