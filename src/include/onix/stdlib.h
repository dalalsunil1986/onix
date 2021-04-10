#ifndef ONIX_STDLIB_H
#define ONIX_STDLIB_H

#include <onix/types.h>
#include <onix/stdio.h>

#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#define BMB BOCHS_MAGIC_BREAKPOINT;
#define PBMB DEBUGF("\n"); BOCHS_MAGIC_BREAKPOINT;

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

u32 round_up(u32 number, u32 size);

#endif