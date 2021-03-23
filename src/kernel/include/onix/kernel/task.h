#ifndef ONIX_THREAD_H
#define ONIX_THREAD_H

#include <onix/types.h>

#define TASK_MAGIC 0x20210323
#define TASK_SIZE 64

#define TASK_INDEX_IDLE 0

typedef void thread_target(void *);

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
    void (*epi)(void);
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

    void (*eip)(thread_target *target, void *args);

    void(*unused_retaddr); // placeholder
    thread_target *target;
    void *args;
    /* data */
} ThreadFrame;

typedef struct Task
{
    u32 *stack;
    TASK_STATUS status;
    u8 priority;
    char name[32];
    u32 stack_magic;
} Task;

extern Task *tasks[TASK_SIZE];
extern Task *current_task;
void init_task();
void schedule();

Task *thread_start(thread_target target, void *args, const char *name, int priority);

#endif