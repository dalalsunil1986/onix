/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/io.h>

void outb(u16 port, u8 value)
{
    asm volatile(
        "out %%al, %%dx\n" ::"d"(port), "a"(value));
}

u8 inb(u16 port)
{
    u8 value = 0;
    asm volatile(
        "in %%dx, %%al\n"
        : "=a"(value)
        : "d"(port));
    return value;
}

void outw(u16 port, u16 value)
{
    asm volatile(
        "out %%ax, %%dx\n" ::"d"(port), "a"(value));
}

u16 inw(u16 port)
{
    u16 value = 0;
    asm volatile(
        "in %%dx, %%ax\n"
        : "=a"(value)
        : "d"(port));
    return value;
}