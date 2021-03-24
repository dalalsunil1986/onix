#include <onix/kernel/task.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/io.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/string.h>
#include <onix/queue.h>

#define DEBUGP DEBUGK
// #define DEBUGP(fmt, args...)

Task *current_task;
Queue tasks_queue;
Queue tasks_ready;

extern void switch_to(Task *current, Task *next);
extern void jump_to_next(Task *next);

static void kernel_thread(thread_target target, void *args)
{
    target(args);
}

void thread_create(Task *thread, thread_target target, void *args)
{
    u32 stack = thread->stack;
    stack -= sizeof(TaskFrame);
    stack -= sizeof(ThreadFrame);
    thread->stack = stack;

    ThreadFrame *frame = (ThreadFrame *)thread->stack;
    frame->eip = kernel_thread;
    frame->target = target;
    frame->args = args;
    frame->ebp = 0;
    frame->ebx = 0;
    frame->esi = 0;
    frame->edi = 0;
}

void thread_init(Task *thread, char *name, int priority)
{
    memset(thread, 0, sizeof(*thread));
    strcpy(thread->name, name);
    thread->status = TASK_INIT;
    thread->priority = priority;
    thread->ticks = thread->priority;
    thread->stack = (u32)thread + PG_SIZE;
    thread->pde = NULL;
    thread->stack_magic = TASK_MAGIC;
}

Task *thread_start(thread_target target, void *args, const char *name, int priority)
{
    Task *thread = page_alloc(USER_KERNEL, 1);
    DEBUGP("Start thread 0x%X node 0x%X\n", (u32)thread, &thread->node);
    thread_init(thread, name, priority);
    thread_create(thread, target, args);
    thread->status = TASK_RUNNING;

    // assert(!queue_find(&tasks_queue, &thread->node));
    // queue_push(&tasks_queue, &thread->node);

    assert(!queue_find(&tasks_ready, &thread->node));
    queue_push(&tasks_ready, &thread->node);

    DEBUGP("Task create 0x%X stack top 0x%X\n", (u32)thread, (u32)thread->stack);
    return thread;
}

void initd_task()
{
    // enable_int();
    while (true)
    {
        show_char('I', 75, 0);
        DEBUGP("init task is running....\n");
    }
}

static void make_init_thread()
{
    Task *thread = (Task *)TASK_MAIN_PAGE;
    thread_init(thread, "init task", 1);
    thread_create(thread, initd_task, NULL);

    // u32 stack = thread->stack;
    // stack += sizeof(ThreadFrame);
    // thread->stack = stack;

    thread->status = TASK_RUNNING;
    DEBUGP("Task create 0x%X stack top 0x%X\n", (u32)thread, (u32)thread->stack);
    current_task = thread;
    // assert(!queue_find(&tasks_queue, &thread->node));
    // queue_push(&tasks_queue, &thread->node);
    jump_to_next(thread);
}

void idle_task()
{
    while (1)
    {
        DEBUGP("idle task is running....\n");
        halt();
    }
}

void test_task()
{
    while (1)
    {
        DEBUGP("test task is running....\n");
    }
}

void init_task()
{
    queue_init(&tasks_queue);
    queue_init(&tasks_ready);

    Task *task = thread_start(test_task, NULL, "test task", 1);
    make_init_thread();
    // Task *idle = thread_start(idle_task, NULL, "idle task", 1);
}

Task *running_thread()
{
    return current_task;
}

void schedule()
{
    Task *cur = running_thread();

    if (cur->status == TASK_RUNNING)
    {
        assert(!queue_find(&tasks_ready, &cur->node));
        queue_push(&tasks_ready, &cur->node);
        cur->status = TASK_READY;
    };
    assert(!queue_empty(&tasks_ready));

    // Task *next = queue_pop(&tasks_ready)->data;
    // DEBUGP("switch 0x%08X to 0x%08X\n", cur, next);

    // current_task = next;
    // switch_to(cur, next);
}
