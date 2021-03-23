#include <onix/kernel/task.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/io.h>
#include <onix/string.h>

Task *tasks[TASK_SIZE];
Task *current_task;

extern void __schedule();

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
    frame->ebp = 1;
    frame->ebx = 2;
    frame->esi = 3;
    frame->edi = 4;
}

void thread_init(Task *thread, char *name, int priority)
{
    memset(thread, 0, sizeof(*thread));
    strcpy(thread->name, name);
    thread->status = TASK_INIT;
    thread->priority = priority;
    thread->ticks = thread->priority;
    thread->stack = (u32)thread + PG_SIZE;
    thread->stack_magic = TASK_MAGIC;
}

Task *thread_start(thread_target target, void *args, const char *name, int priority)
{
    Task *thread = page_alloc(USER_KERNEL, 1);
    // DEBUGK("Start thread 0x%X\n", (u32)thread);
    thread_init(thread, name, priority);
    thread_create(thread, target, args);
    thread->status = TASK_RUNNING;
    DEBUGK("Task create 0x%X stack top 0x%X\n", (u32)thread, (u32)thread->stack);
    return thread;
}

void idle_task()
{
    while (1)
    {
        DEBUGK("idle task is running....\n");
        halt();
    }
}

void init_task()
{
    current_task = NULL;
    for (size_t i = 0; i < TASK_SIZE; i++)
    {
        tasks[i] = NULL;
    }
    Task *idle = thread_start(idle_task, NULL, "idle task", 1);
    tasks[TASK_INDEX_IDLE] = idle;
    current_task = idle;
}

void schedule()
{
    if(current_task->status == TASK_RUNNING);
    // current_task->ticks--;
    // DEBUGK("Task 0x%X stack top 0x%X\n", (u32)current_task, (u32)current_task->stack);
    // __schedule();
    // Task *task = current_task;
}
