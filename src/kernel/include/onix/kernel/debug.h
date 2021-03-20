#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

#ifdef ONIX_DEBUG
#include <onix/kernel/printk.h>
#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#define DEBUGK(fmt, args...) printk(fmt, ##args)
#else
#define BOCHS_MAGIC_BREAKPOINT
#define DEBUGK(fmt, args...)
#endif

#define BMB BOCHS_MAGIC_BREAKPOINT

#endif