#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/queue.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/arena.h>
#include <fs/file.h>

#define TASK_MAGIC 0x20210323
#define TASK_INIT_PAGE (0x10000 | KERNEL_ADDR_MASK)

#define TASK_INDEX_IDLE 0
#define TASK_INDEX_INIT 0

#define TASK_MAX_OPEN_FILES 32

typedef u32 Tasktarget(void *);

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

typedef struct InterruptFrame
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
} InterruptFrame;

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

typedef struct ProcessFrame
{
    u32 ebp;
    u32 ebx;
    u32 edi;
    u32 esi;
    void (*eip)(Tasktarget *target, void *args);
} ProcessFrame;

typedef struct Task
{
    u32 *stack;
    Node node;
    Node all_node;
    TASK_STATUS status;
    u8 priority;
    u32 tid;
    u32 pid;
    u32 ppid;
    u32 ticks;
    char name[32];
    u8 user; // 0 表示内核，其他表示用户
    Vaddr vaddr;
    PageTable pde;
    ArenaDesc adesc[DESC_COUNT];
    fd_t file_table[TASK_MAX_OPEN_FILES];
    char *cwd;
    u32 message;
    u32 exit_code;
    u32 magic;
} Task;

void init_tasks();
void schedule();

void make_setup_task();
Task *running_task();

void push_task(Task *task);
void push_ready_task(Task *task);

void task_init(Task *task, char *name, int priority, int user);
void task_create(Task *task, Tasktarget target, void *args);
Task *task_start(Tasktarget target, void *args, const char *name, int priority);

void task_hanging(Task *task);
void task_block(Task *task);
void task_unblock(Task *task);
void task_yield();

u32 task_fork();

void task_exit(Task *task);
void task_destory(Task *task);

fd_t task_install_fd(fd_t fd);
fd_t task_global_fd(fd_t fd);

#endif