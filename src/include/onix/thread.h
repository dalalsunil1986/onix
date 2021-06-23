/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#ifndef ONIX_THREAD_H
#define ONIX_THREAD_H

#include <onix/types.h>
#include <onix/memory.h>
#include <onix/list.h>

#define THREAD_MAGIC 0x20210623

#define KERNEL_USER 0

typedef u32 thread_target_t(int argc, char const *argv[]);

typedef enum THREAD_STATUS
{
    INIT,
    RUNNING,
    READY,
    BLOCKED,
    WAITING,
    FORKING,
    HANGING,
    DIED,
} THREAD_STATUS;

typedef struct intr_frame_t
{
    u32 vector;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp_dummy;
    // 虽然 pushad 把esp 也压入，但esp 是不断变化的，所以会被 popad 忽略

    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 gs;
    u32 fs;
    u32 es;
    u32 ds;

    u32 error;

    void (*eip)(void);
    u32 cs;
    u32 eflags;
    void *esp;
    u32 ss;

} intr_frame_t;

typedef struct thread_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 ebx;
    void (*eip)(void);
} thread_frame_t;

typedef struct thread_args_t
{
    void (*eip)(void);
    void (*target)(void);
    u32 argc;
    void *argv;
} thread_args_t;

typedef struct thread_t
{
    u32 *stack;
    list_node_t node;

    u8 status;
    u32 priority;
    u32 ticks;
    char name[32];
    u32 user;
    page_table_t pde;
    addr_t vaddr;
    u32 magic;
} thread_t;

extern thread_t *current_thread();

thread_t *thread_init(thread_target_t target, int argc, char const *argv, char *name, int priority, u32 user);

extern void thread_switch(thread_t *thread);
void schedule();

void init_thread();

#endif