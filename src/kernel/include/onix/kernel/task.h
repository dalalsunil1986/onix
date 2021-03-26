#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/queue.h>
#include <onix/kernel/memory.h>

#define TASK_MAGIC 0x20210323
#define TASK_INIT_PAGE (0x10000 | KERNEL_ADDR_MASK)

#define TASK_INDEX_IDLE 0
#define TASK_INDEX_INIT 0

typedef void Tasktarget(void *);

typedef enum TASK_STATUS
{
    TASK_INIT,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED,
} TASK_STATUS;

typedef struct TaskFrame
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
} TaskFrame;

typedef struct ThreadFrame
{
    u32 ebp;
    u32 ebx;
    u32 edi;
    u32 esi;

    void (*eip)(Tasktarget *target, void *args);
    void(*addr); // placeholder

    Tasktarget *target;
    void *args;
    /* data */
} ThreadFrame;

typedef struct Task
{
    u32 *stack;
    Node node;
    Node queue_node;
    TASK_STATUS status;
    u8 priority;
    u32 ticks;
    char name[32];
    PageTable pde;
    u32 magic;
} Task;

void init_task();
void schedule();

Task *running_task();
Task *task_start(Tasktarget target, void *args, const char *name, int priority);

#endif