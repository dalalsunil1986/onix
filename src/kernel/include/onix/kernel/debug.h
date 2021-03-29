#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

void debugk(const char *fmt, ...);

#ifdef ONIX_DEBUG
#include <onix/kernel/printk.h>
#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)
#else
#define BOCHS_MAGIC_BREAKPOINT
#define DEBUGK(fmt, args...)
#endif

#define BMB DEBUGK("\n"); BOCHS_MAGIC_BREAKPOINT;

#endif