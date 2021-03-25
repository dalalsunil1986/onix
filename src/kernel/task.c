#include <onix/kernel/task.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/io.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/string.h>
#include <onix/queue.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Task *current_task;
Queue tasks_queue;
Queue tasks_ready;

extern void restart(Task *init);
extern void switch_to(Task *current, Task *next);
void idle_task();

Task *running_task()
{
    return current_task;
}

Task *pop_ready_task()
{
    assert(!queue_empty(&tasks_ready));
    Task *task = element_entry(Task, node, queue_popback(&tasks_ready));
    assert(task->magic == TASK_MAGIC);
    return task;
}

static void kernel_task(Tasktarget target, void *args)
{
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

    frame->eflags = 0x1202;
    frame->cs = SELECT_KERNEL_CODE_INDEX << 3 | PL0;
    frame->ebp = 0;
    frame->ebx = 0;
    frame->esi = 0;
    frame->edi = 0;
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

    DEBUGP("Task create 0x%X stack top 0x%X\n", (u32)task, (u32)task->stack);
    return task;
}

void init_kernel_task()
{
    Task *idle = task_start(idle_task, NULL, "idle task", 1);
    DEBUGP("Create idle finish\n");
    u32 counter = 0;
    while (true)
    {
        DEBUGP("init task....\n");
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
        schedule();
    }
}

static void make_init_task()
{
    Task *task = (Task *)TASK_MAIN_PAGE;
    task_init(task, "init task", 5);
    task_create(task, init_kernel_task, NULL);
    task->status = TASK_RUNNING;
    assert(task->magic == TASK_MAGIC);

    assert(!queue_find(&tasks_queue, &task->queue_node));
    queue_push(&tasks_queue, &task->queue_node);

    DEBUGP("Task create 0x%X stack top 0x%X\n", (u32)task, (u32)task->stack);
    current_task = task;
    assert(current_task->magic == TASK_MAGIC);
}

void idle_task()
{
    u32 counter = 0;
    while (true)
    {
        DEBUGP("idle task....\n");
        // BMB;
        Task *task = running_task();
        assert(task->magic == TASK_MAGIC);
        counter++;
        char ch = ' ';
        if ((counter % 2) != 0)
        {
            ch = 'D';
        }
        show_char(ch, 75, 0);
        halt();
        schedule();
    }
}

void test_task()
{
    while (1)
    {
        DEBUGP("test task is running....\n");
    }
}

void schedule()
{
    bool inter = get_interrupt_status();

    Task *cur = running_task();
    assert(cur->magic == TASK_MAGIC);

    char ch = ' ';
    if ((cur->ticks % 2) != 0)
    {
        ch = 'S';
    }
    // char ch = cur->ticks % 10 + 0x30;
    show_char(ch, 73, 0);

    if (cur->status == TASK_RUNNING)
    {
        assert(!queue_find(&tasks_ready, &cur->node));
        queue_push(&tasks_ready, &cur->node);
        cur->status = TASK_READY;
    };

    Task *next = pop_ready_task();
    assert(next->magic == TASK_MAGIC);
    next->status = TASK_RUNNING;

    assert(((u32)next & 0xfff) == 0);
    current_task = next;

    if (next == cur)
        return;

    if (true)
    {
        DEBUGP("switch 0x%08X to 0x%08X\n", cur, next);
        switch_to(cur, next);
    }
    else
    {
        // DEBUGP("exit 0x%08X to 0x%08X\n", cur, next);
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
}