
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
    // pid_t pid = sys_fork();
    // if (pid)
    // {
    //     printf("this is parent wait!!!\n");
    //     int status;
    //     sys_wait(&status);
    //     printf("this is parent wait finish %d!!!\n", status);
    // }
    // else
    // {
    //     printf("This is child!!!\n");
    //     sys_exit(0);
    //     printf("this is child exited!!!\n");
    // }

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

        task = task_status_task(TASK_DIED);
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
        task_block(task, TASK_BLOCKED);
        pause();
    }
}

extern Task *process_copy(Task *parent);

void fork_task()
{
    while (true)
    {
        Task *parent = task_status_task(TASK_FORKING);
        if (parent == NULL)
        {
            task_yield();
            continue;
        }
        Task *task = process_copy(parent);
        parent->message = task->pid;
        push_ready_task(parent);
        push_ready_task(task);
        push_task(task);
    }
}

void join_task()
{
    while (true)
    {
        Task *parent = task_status_task(TASK_WAITING);
        if (parent == NULL)
        {
            task_yield();
            continue;
        }

        Task *child = task_child_task(parent->pid, NULL);

        if (child == NULL)
        {
            DEBUGP("There is no child for %s\n", parent->name);
            parent->message = -1;
            push_ready_task(parent);
            continue;
        }
        while (child != NULL)
        {
            if (child->status == TASK_HANGING)
            {
                DEBUGP("task hanging %s \n", child->name);
                parent->message = child->exit_code;
                task_block(child, TASK_DIED);
                push_ready_task(parent);
                break;
            }
            child = task_child_task(parent->pid, child);
        }
    }
}

void test_task()
{
    while (true)
    {
        task_yield();
    }
}