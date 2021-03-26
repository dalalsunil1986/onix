#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

extern void outb(u16 port, u8 value);
extern u8 inb(u16 port);
extern void halt();
extern void pause();

#endif