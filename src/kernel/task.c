#include <onix/kernel/task.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/io.h>
#include <onix/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/string.h>
#include <onix/queue.h>
#include <onix/kernel/ioqueue.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/mutex.h>
#include <onix/kernel/clock.h>
#include <onix/kernel/pid.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Queue tasks_queue;
Queue tasks_ready;
Queue tasks_died;

u32 init_stack_top;

extern void switch_to(Task *current, Task *next);
extern void process_activate(Task *next);

extern void idle_task();
extern void init_kernel_task();
extern void keyboard_task();

static Task *idle;

void push_task(Task *task)
{
    bool old = disable_int();
    queue_push(&tasks_queue, &task->all_node);
    set_interrupt_status(old);
}

Task *pop_ready_task()
{
    bool old = disable_int();

    if (queue_empty(&tasks_ready))
    {
        push_ready_task(idle);
    }

    assert(!queue_empty(&tasks_ready));
    Task *task = element_entry(Task, node, queue_popback(&tasks_ready));
    assert(task->magic == TASK_MAGIC);
    set_interrupt_status(old);
    return task;
}

void push_ready_task(Task *task)
{
    bool old = disable_int();
    task->status = TASK_READY;
    assert(!queue_find(&tasks_ready, &task->node));
    queue_push(&tasks_ready, &task->node);
    set_interrupt_status(old);
}

Task *pop_died_task()
{
    if (queue_empty(&tasks_died))
        return NULL;
    assert(!queue_empty(&tasks_ready));
    Task *task = element_entry(Task, node, queue_popback(&tasks_died));
    assert(task->magic == TASK_MAGIC);
    return task;
}

void kernel_task(Tasktarget target, void *args)
{
    assert(!get_interrupt_status());
    enable_int();
    target(args);
    task_exit(running_task());
}

void task_create(Task *task, Tasktarget target, void *args)
{
    u32 stack = task->stack;
    stack -= sizeof(TaskFrame);
    stack -= sizeof(ThreadFrame);
    task->stack = stack;

    ThreadFrame *frame = (ThreadFrame *)task->stack;
    frame->eip = kernel_task;
    frame->target = target;
    frame->args = args;
    frame->ebp = 0x11111111; // 这里的值不重要，用于调试定位栈顶信息
    frame->ebx = 0x22222222;
    frame->edi = 0x33333333;
    frame->esi = 0x44444444;
    frame->addr = 0x55555555;
}

void init_file_table(Task *task)
{
    task->file_table[0] = onix_stdin;
    task->file_table[1] = onix_stdout;
    task->file_table[2] = onix_stderr;

    u8 idx = 3;
    while (idx < TASK_MAX_OPEN_FILES)
    {
        task->file_table[idx] = FILE_NULL;
        idx++;
    }
}

fd_t task_install_fd(fd_t fd)
{
    Task *task = running_task();
    fd_t idx = 3;
    while (idx < TASK_MAX_OPEN_FILES)
    {
        if (task->file_table[idx] == FILE_NULL)
        {
            task->file_table[idx] = fd;
            break;
        }
        idx++;
    }
    if (idx == TASK_MAX_OPEN_FILES)
    {
        printk("Exceed task max open files\n");
        return FILE_NULL;
    }
    return idx;
}

fd_t task_global_fd(fd_t fd)
{
    assert(fd >= 0 && fd < TASK_MAX_OPEN_FILES);
    Task *task = running_task();
    fd_t global_fd = task->file_table[fd];
    assert(global_fd >= 0 && global_fd < MAX_OPEN_FILES);
    return global_fd;
}

void task_init(Task *task, char *name, int priority, int user)
{
    Task *cur = running_task();

    memset(task, 0, sizeof(*task));
    strcpy(task->name, name);
    task->tid = allocate_pid();
    task->pid = cur->pid;
    task->status = TASK_INIT;
    task->priority = priority;
    task->ticks = task->priority;
    task->stack = (u32)task + PG_SIZE;
    task->pde = NULL;
    task->magic = TASK_MAGIC;
    task->user = user;
    task->cwd = NULL;

    init_arena_desc(task->adesc);
    init_file_table(task);

    if (task != cur)
    {
        task->vaddr.start = cur->vaddr.start;
        task->vaddr.mmap.bits = cur->vaddr.mmap.bits;
        task->vaddr.mmap.length = cur->vaddr.mmap.length;
        task->pde = cur->pde;
    }
}

Task *task_start(Tasktarget target, void *args, const char *name, int priority)
{
    Task *cur = running_task();
    // BMB;
    Task *task = page_alloc(1);
    DEBUGP("Start task 0x%X\n", (u32)task);
    task_init(task, name, priority, cur->user);
    task_create(task, target, args);

    task->cwd = malloc(MAX_PATH_LEN);

    memset(task->cwd, 0, MAX_PATH_LEN);
    memcpy(task->cwd, "/", 1);

    assert(!queue_find(&tasks_queue, &task->all_node));
    push_task(task);

    assert(task->magic == TASK_MAGIC);
    assert(!queue_find(&tasks_ready, &task->node));
    push_ready_task(task);

    DEBUGP("Task create %s 0x%X stack top 0x%X\n", name, (u32)task, (u32)task->stack);
    return task;
}

void task_block(Task *task)
{
    bool old = disable_int();
    Task *cur = running_task();
    assert(task->status != TASK_BLOCKED);
    task->status = TASK_BLOCKED;

    if (cur == task)
    {
        schedule();
    }

    set_interrupt_status(old);
}

void task_unblock(Task *task)
{
    bool old = disable_int();
    assert(task->status == TASK_BLOCKED);
    push_ready_task(task);
    set_interrupt_status(old);
}

void task_yield()
{
    Task *task = running_task();

    // DEBUGP("yield task 0x%X\n", task);

    bool old = disable_int();
    assert(!queue_find(&tasks_ready, &task->node));

    push_ready_task(task);
    schedule();
    set_interrupt_status(old);
}

u32 task_fork()
{
    DEBUGP("Fork task...\n");
}

bool task_check_tid(Node *node, pid_t pid)
{
    Task *task = element_entry(Task, all_node, node);
    if (task->pid == pid)
        return true;
    return false;
}

Task *task_found(pid_t tid)
{
    Node *node = queue_traversal(&tasks_queue, task_check_tid, tid);
    if (!node)
        return NULL;
    Task *task = element_entry(Task, all_node, node);
    return task;
}

void task_exit(Task *task)
{
    DEBUGP("Task exit 0x%08X\n", task);
    bool old = disable_int();
    task->status = TASK_DIED;
    if (queue_find(&tasks_ready, &task->node))
    {
        queue_remove(&tasks_ready, &task->node);
    }
    queue_remove(&tasks_queue, &task->all_node);
    release_pid(task->tid);
    assert(!queue_find(&tasks_died, &task->node));
    queue_push(&tasks_died, &task->node);
    schedule();
}

void task_destory(Task *task)
{
    DEBUGP("free pages %d\n", free_pages);
    if (task->pid == task->tid)
    {
        free_user_pde(task);
        page_free(task->vaddr.mmap.bits, 1);
    }
    DEBUGP("free task page 0x%08X\n", task);
    if (task->cwd != NULL)
    {
        free(task->cwd);
        task->cwd = NULL;
    }
    page_free(task, 1);
    DEBUGP("free pages %d tasks %d died %d\n", free_pages, tasks_queue.size, tasks_died.size);
}

static void make_init_task()
{
    Task *task = (Task *)TASK_INIT_PAGE;
    task_init(task, "init task", 50, USER_KERNEL);
    task_create(task, init_kernel_task, NULL);

    ThreadFrame *frame = (ThreadFrame *)task->stack;
    frame->ebp = 0x1111; // 这里的值不重要，用于调试定位栈顶信息
    frame->ebx = 0x2222;
    frame->edi = 0x3333;
    frame->esi = 0x4444;
    frame->addr = 0x5555;

    init_stack_top = (u32)task->stack + element_offset(ThreadFrame, addr);
    // 跳过 switch_to 返回的栈空间
    // 这样从形式上和其他线程的栈空间一致

    task->status = TASK_RUNNING;
    assert(task->magic == TASK_MAGIC);

    assert(!queue_find(&tasks_queue, &task->all_node));
    push_task(task);
    DEBUGP("Task create 0x%X stack top 0x%X target 0x%08X\n",
           (u32)task, (u32)task->stack, init_kernel_task);
}

void make_setup_task()
{
    Task *task = running_task();
    task_init(task, "init task", 50, USER_KERNEL);
    task_create(task, NULL, NULL);

#ifndef ONIX_KERNEL_DEBUG
    task->cwd = (char *)(0x12000);
#else
    task->cwd = malloc(MAX_PATH_LEN);
#endif

    memset(task->cwd, 0, MAX_PATH_LEN);
    memcpy(task->cwd, "/", 1);

    task->status = TASK_RUNNING;
    assert(task->magic == TASK_MAGIC);

    DEBUGP("Task create 0x%X stack top 0x%X target 0x%08X\n", (u32)task, (u32)task->stack, NULL);
}

void schedule()
{
    assert(!get_interrupt_status());

    Task *cur = running_task();
    assert(cur->magic == TASK_MAGIC);

    if (cur->status == TASK_RUNNING)
    {
        assert(!queue_find(&tasks_ready, &cur->node));
        push_ready_task(cur);
        cur->status = TASK_READY;
    }

    Task *next = pop_ready_task();
    assert(next->magic == TASK_MAGIC);
    next->status = TASK_RUNNING;

    assert(((u32)next & 0xfff) == 0);

    // PBMB;
    if (next == cur)
        return;
    // DEBUGP("Task schedule 0x%X name %s\n", next, next->name);
    // BMB;
    process_activate(next);
    // PBMB;

    show_char(next->tid % 10 + 0x30, 73, 0);
    switch_to(cur, next);
}

void init_task()
{
    CHECK_STACK;
    DEBUGP("Size Taskframe %d\n", sizeof(TaskFrame));
    DEBUGP("Size Threadframe %d\n", sizeof(ThreadFrame));
    DEBUGP("StackFrame size 0x%X\n", sizeof(ThreadFrame) + sizeof(TaskFrame));
    queue_init(&tasks_queue);
    queue_init(&tasks_ready);
    queue_init(&tasks_died);
    init_pid();

    make_init_task();
    idle = task_start(idle_task, NULL, "idle task", 1);
    task_start(keyboard_task, NULL, "key task", 16);
}
