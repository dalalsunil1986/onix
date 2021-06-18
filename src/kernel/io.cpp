#include <onix/io.h>

void onix::io::outb(u16 port, u8 value)
{
    asm volatile(
        "out %%al, %%dx\n" ::"d"(port), "a"(value));
}

u8 onix::io::inb(u16 port)
{
    u8 value = 0;
    asm volatile(
        "in %%dx, %%al\n"
        : "=a"(value)
        : "d"(port));
    return value;
}

void onix::io::outw(u16 port, u16 value)
{
    asm volatile(
        "out %%ax, %%dx\n" ::"d"(port), "a"(value));
}

u16 onix::io::inw(u16 port)
{
    u16 value = 0;
    asm volatile(
        "in %%dx, %%ax\n"
        : "=a"(value)
        : "d"(port));
    return value;
}