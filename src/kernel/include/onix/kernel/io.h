#ifndef ONIX_IO_H
#define ONIX_IO_H

#include <onix/types.h>

extern void outb(u16 port, u8 value);
extern void outsw(u16 port, const void *addr, u32 size);
extern void outsd(u16 port, const void *addr, u32 size);
extern u8 inb(u16 port);
extern void insw(u16 port, const void *addr, u32 size);
extern void insd(u16 port, const void *addr, u32 size);
extern void halt();
extern void stop();
extern void pause();

#endif