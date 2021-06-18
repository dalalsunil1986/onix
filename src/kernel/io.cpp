#include <onix/kernel/io.h>

void onix::kernel::outb(u16 port, u8 value)
{
    asm volatile(
        "out %%al, %%dx\n" ::"d"(port), "a"(value));
}

u8 onix::kernel::inb(u16 port)
{
    u8 value = 0;
    asm volatile(
        "in %%dx, %%al\n"
        : "=a"(value)
        : "d"(port));
    return value;
}

void onix::kernel::outw(u16 port, u16 value)
{
    asm volatile(
        "out %%ax, %%dx\n" ::"d"(port), "a"(value));
}

u16 onix::kernel::inw(u16 port)
{
    u16 value = 0;
    asm volatile(
        "in %%dx, %%ax\n"
        : "=a"(value)
        : "d"(port));
    return value;
}