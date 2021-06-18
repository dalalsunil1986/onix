/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-18
*/

#pragma once
#include <onix/types.h>

namespace onix
{
    namespace io
    {
        void outb(u16 port, u8 value);
        u8 inb(u16 port);
        void outw(u16 port, u16 value);
        u16 inw(u16 port);
    }
}
