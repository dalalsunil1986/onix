#include <onix/kernel/task.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/io.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/string.h>
#include <onix/queue.h>
#include <onix/kernel/ioqueue.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern IOQueue key_ioq;

Queue tasks_queue;
Queue tasks_ready;

u32 init_stack_top;

extern void restart(Task *init);
extern void switch_to(Task *current, Task *next);
void idle_task();

Task *pop_ready_task()
{
    assert(!queue_empty(&tasks_ready));
    Task *task = element_entry(Task, node, queue_popback(&tasks_ready));
    assert(task->magic == TASK_MAGIC);
    return task;
}

void kernel_task(Tasktarget target, void *args)
{
    assert(!get_interrupt_status());
    enable_int();
    target(args);
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

void task_init(Task *task, char *name, int priority)
{
    memset(task, 0, sizeof(*task));
    strcpy(task->name, name);
    task->status = TASK_INIT;
    task->priority = priority;
    task->ticks = task->priority;
    task->stack = (u32)task + PG_SIZE;
    task->pde = NULL;
    task->magic = TASK_MAGIC;
}

Task *task_start(Tasktarget target, void *args, const char *name, int priority)
{
    Task *task = page_alloc(USER_KERNEL, 1);
    DEBUGP("Start task 0x%X\n", (u32)task);
    task_init(task, name, priority);
    task_create(task, target, args);
    task->status = TASK_READY;

    assert(!queue_find(&tasks_queue, &task->queue_node));
    queue_push(&tasks_queue, &task->queue_node);

    assert(task->magic == TASK_MAGIC);
    assert(!queue_find(&tasks_ready, &task->node));
    queue_push(&tasks_ready, &task->node);

    DEBUGP("Task create 0x%X stack top 0x%X target 0x%08x\n", (u32)task, (u32)task->stack, kernel_task);
    return task;
}

void task_block(Task *task)
{
    bool old = disable_int();

    assert(task->status != TASK_BLOCKED);
    task->stack = TASK_BLOCKED;
    schedule();

    set_interrupt_status(old);
}

void task_unblock(Task *task)
{
    bool old = disable_int();
    assert(task->status == TASK_BLOCKED);
    task->status = TASK_READY;
    schedule();

    set_interrupt_status(old);
}

void init_kernel_task()
{
    u32 counter = 0;
    while (true)
    {
        // BMB;
        // DEBUGP("init task....\n");
        Task *task = running_task();
        assert(task->magic == TASK_MAGIC);
        // BMB;
        counter++;
        char ch = ' ';
        if ((counter % 2) != 0)
        {
            ch = 'K';
        }
        show_char(ch, 77, 0);
    }
}

void idle_task()
{
    static u32 idle_counter = 0;
    while (true)
    {
        // BMB;
        // DEBUGP("idle task 0x%X....\n", idle_counter);
        Task *task = running_task();
        assert(task->magic == TASK_MAGIC);
        idle_counter++;
        char ch = ' ';
        if ((idle_counter % 2) != 0)
        {
            ch = 'D';
        }
        show_char(ch, 75, 0);
        pause();
        schedule();
    }
}

static void make_init_task()
{
    Task *task = (Task *)TASK_INIT_PAGE;
    task_init(task, "init task", 50);
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

    assert(!queue_find(&tasks_queue, &task->queue_node));
    queue_push(&tasks_queue, &task->queue_node);
    DEBUGP("Task create 0x%X stack top 0x%X target 0x%08X\n", (u32)task, (u32)task->stack, init_kernel_task);
}

void schedule()
{
    assert(!get_interrupt_status());

    Task *cur = running_task();
    assert(cur->magic == TASK_MAGIC);

    if (cur->status == TASK_RUNNING)
    {
        assert(!queue_find(&tasks_ready, &cur->node));
        queue_push(&tasks_ready, &cur->node);
        cur->status = TASK_READY;
    }

    Task *next = pop_ready_task();
    assert(next->magic == TASK_MAGIC);
    next->status = TASK_RUNNING;

    assert(((u32)next & 0xfff) == 0);

    if (next == cur)
        return;

    switch_to(cur, next);
}

void keyboard_task()
{
    while (1)
    {
        bool old = disable_int();
        if (!ioqueue_empty(&key_ioq))
        {
            char byte = ioqueue_get(&key_ioq);
            put_char(byte);
        }
        set_interrupt_status(old);
    }
}

void init_task()
{
    DEBUGP("Size Taskframe %d\n", sizeof(TaskFrame));
    DEBUGP("Size Threadframe %d\n", sizeof(ThreadFrame));
    DEBUGP("StackFrame size 0x%X\n", sizeof(ThreadFrame) + sizeof(TaskFrame));
    queue_init(&tasks_queue);
    queue_init(&tasks_ready);
    make_init_task();
    task_start(idle_task, NULL, "idle task", 1);
    task_start(keyboard_task, NULL, "key task", 16);
}
