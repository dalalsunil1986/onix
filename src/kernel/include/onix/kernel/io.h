#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

extern void io_outb(u16 port, u8 value);
extern u8 io_inb(u16 port);

#endif