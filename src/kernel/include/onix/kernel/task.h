#ifndef ONIX_TASK_H
#define ONIX_TASK_H

#include <onix/types.h>
#include <onix/queue.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/arena.h>
#include <onix/kernel/pid.h>
#include <fs/file.h>

#define TASK_MAGIC 0x20210323
#define TASK_INIT_PAGE (0x10000 | KERNEL_ADDR_MASK)

#define TASK_INDEX_IDLE 0
#define TASK_INDEX_INIT 0

#define TASK_MAX_OPEN_FILES 32

typedef u32 Tasktarget(int argc, char const *argv[]);

typedef enum TASK_STATUS
{
    TASK_INIT,
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_FORKING,
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

typedef struct TaskFrame
{
    u32 ebp;
    u32 ebx;
    u32 edi;
    u32 esi;
    void (*eip)(Tasktarget *target, void *args);
} TaskFrame;

typedef struct TaskArgs
{
    void (*eip)(void); // 用于模拟调用栈 wrapper
    void (*target)(void);
    u32 argc;
    void *argv;
} TaskArgs;

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
void start_tasks();
void schedule();

void init_setup_task();
Task *running_task();

void push_task(Task *task);
void push_ready_task(Task *task);
Task *task_status_task(TASK_STATUS status);
Task *task_child_task(pid_t pid, Task *child);

void task_init(Task *task, char *name, int priority, int user);
void task_create(Task *task, Tasktarget target, int argc, char const *argv[]);
Task *task_start(Tasktarget target, int argc, char const *argv[], const char *name, int priority);

void task_hanging(Task *task);
void task_block(Task *task, TASK_STATUS status);
void task_unblock(Task *task);
void task_yield();

void task_wrapper(Tasktarget target, int argc, char const *argv[]);
u32 task_fork();

void task_exit(Task *task);
void task_destory(Task *task);

fd_t task_install_fd(fd_t fd);
fd_t task_global_fd(fd_t fd);

#endif