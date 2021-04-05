#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

void debugk(const char *fmt, ...);

#ifdef ONIX_DEBUG
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)
#define CHECK_STACK assert(running_task()->magic == TASK_MAGIC);
#else
#define BOCHS_MAGIC_BREAKPOINT
#define DEBUGK(fmt, args...)
#define CHECK_STACK
#endif

#define BMB BOCHS_MAGIC_BREAKPOINT;
#define PBMB DEBUGK("\n"); BOCHS_MAGIC_BREAKPOINT;

#endif