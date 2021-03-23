#ifndef ONIX_THREAD_H
#define ONIX_THREAD_H

#include <onix/types.h>

typedef void thread_target(void *);

typedef enum TASK_STATUS
{
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

    void (*epi)(thread_target *target, void *args);

    void(*unused_retaddr); // placeholder
    thread_target *target;
    void *args;
    /* data */
} ThreadFrame;

typedef struct Task
{
    u8 id;
    u32 *stack;
    TASK_STATUS status;
    u8 priority;
    char name[32];
    u32 stack_magic;
} Task;

// typedef void thread_create(Task);
#endif