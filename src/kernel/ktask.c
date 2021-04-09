
#include <onix/kernel/task.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/process.h>
#include <onix/assert.h>
#include <onix/stdio.h>
#include <onix/syscall.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern int osh_task();

void init_task()
{
    clear();
    u32 pid = sys_fork();
    if (pid)
    {
        u32 counter = 0;
        while (true)
        {
            counter++;
            char ch = ' ';
            if ((counter % 2) != 0)
            {
                ch = 'K';
            }
            show_char(ch, 79, 0);
            sys_sleep(1000);
        }
    }
    else
    {
        osh_task(0, NULL);
    }
}

void sweep_task()
{
    Task *task;
    while (true)
    {
        task = running_task();
        assert(task->magic == TASK_MAGIC);

        task = pop_died_task();
        if (task != NULL)
        {
            task_destory(task);
        }
        task_yield();
    }
}

void idle_task()
{
    while (true)
    {
        Task *task = running_task();
        assert(task->magic == TASK_MAGIC);
        task_block(task);
        pause();
    }
}

extern Task *pop_fork_task();
extern Task *process_copy(Task *parent);

void fork_task()
{
    while (true)
    {
        Task *parent = pop_fork_task();
        if (parent == NULL)
        {
            task_yield();
            continue;
        }
        Task *task = process_copy(parent);
        parent->message = task->pid;
        push_ready_task(parent);
        push_ready_task(task);
    }
}

void test_task()
{
    while (true)
    {
        // print_irq_mask();
        // task_hanging(running_task());
        task_yield();
    }
}