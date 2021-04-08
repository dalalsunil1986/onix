
#include <onix/kernel/task.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/assert.h>
#include <onix/kernel/keyboard.h>
#include <onix/kernel/ioqueue.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern IOQueue key_ioq;

void init_task()
{
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
        task_block(task);
        pause();
    }
}

void keyboard_task()
{
    while (true)
    {
        bool old = disable_int();
        if (!ioqueue_empty(&key_ioq))
        {
            char byte = ioqueue_get(&key_ioq);
            put_char(byte);
        }
        clock_sleep(1);
        set_interrupt_status(old);
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