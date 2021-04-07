#ifndef ONIX_PROCESS_H
#define ONIX_PROCESS_H

#include <onix/types.h>
#include <onix/kernel/task.h>

#define DEFAULT_PRIORITY 100

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

typedef struct ProcessArgs
{
    void (*eip)(void);
    void (*target)(void);
    void *args;
} ProcessArgs;

static TSS tss;

void init_process();
void init_tss();

void process_activate(Task *task);
void process_start(Tasktarget target, void *args);
void process_execute(Tasktarget *target, const char *name);

void test_process();

#endif