#ifndef ONIX_PROCESS_H
#define ONIX_PROCESS_H

#include <onix/types.h>

typedef struct TSS
{
    u32 backlink;
    u32 *esp0;
    u32 ss0;
    u32 *esp1;
    u32 ss1;
    u32 *esp2;
    u32 ss2;
    u32 cr3;
    u32 (*eip)(void);
    u32 flags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trace;
    u16 iobase;
} TSS;

static TSS tss;


void init_process();
void init_tss();

#endif