#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

#ifdef ONIX_DEBUG
#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#else
#define BOCHS_MAGIC_BREAKPOINT
#endif

#define BMB BOCHS_MAGIC_BREAKPOINT

#endif