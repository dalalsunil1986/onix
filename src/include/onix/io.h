/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

u8 inb(u16 port);
u16 inw(u16 port);

void outb(u16 port, u8 value);
void outw(u16 port, u16 value);

#endif