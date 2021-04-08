
#include <onix/kernel/task.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/process.h>
#include <onix/assert.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern int osh_task(int argc, char const *argv[]);

void init_task()
{
    __clear();

    process_execute(osh_task, 0, NULL, "osh");

    u32 counter = 0;
    Task *task;

    while (true)
    {
        // BMB;
        // DEBUGP("init task....\n");
        task = running_task();
        assert(task->magic == TASK_MAGIC);
        // BMB;
        counter++;
        char ch = ' ';
        if ((counter % 2) != 0)
        {
            ch = 'K';
        }
        show_char(ch, 77, 0);

        task = pop_died_task();
        if (task != NULL)
        {
            task_destory(task);
        }
        clock_sleep(100);
        pause();
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