#pragma once
#include <onix/types.h>

namespace onix
{
    namespace kernel
    {
        void outb(u16 port, u8 value);
        u8 inb(u16 port);
        void outw(u16 port, u16 value);
        u16 inw(u16 port);
    }
}
