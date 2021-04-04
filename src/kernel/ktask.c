
#include <onix/kernel/task.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/keyboard.h>
#include <onix/kernel/ioqueue.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern IOQueue key_ioq;

extern void test_process();
extern void init_fs();

void init_kernel_task()
{
    // __clear();
    // init_harddisk();
    // test_process();
    // init_fs();

    u32 counter = 0;
    Task *task;

    // bool old = disable_int();
    // while (tasks_queue.size < 100)
    // {
    //     task_start(test_task, NULL, "test task", 16);
    //     clock_sleep(1);
    //     DEBUGP("free pages 0x%d task size %d\n", free_pages, tasks_queue.size);
    //     continue;
    // }
    // set_interrupt_status(old);

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
    while (1)
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