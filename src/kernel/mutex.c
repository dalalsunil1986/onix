#include <onix/kernel/mutex.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

void sema_init(Semaphore *sema, u8 value)
{
    sema->value = value;
    queue_init(&sema->waiters);
}

void lock_init(Lock *lock)
{
    lock->holder = NULL;
    lock->repeat = 0;
    sema_init(&lock->sema, 1);
}

void sema_down(Semaphore *sema)
{
    bool old = disable_int();
    while (sema->value == 0)
    {
        Task *task = running_task();
        assert(!queue_find(&sema->waiters, &task->node));
        if (queue_find(&sema->waiters, &task->node))
        {
            panic("sema down: task bloced has been in waiters list\n");
        }
        queue_push(&sema->waiters, &task->node);
        task_block(task);
    }
    sema->value--;
    assert(sema->value == 0);
    set_interrupt_status(old);
}

void sema_up(Semaphore *sema)
{
    bool old = disable_int();
    assert(sema->value == 0);
    if (!queue_empty(&sema->waiters))
    {
        Task *task = element_entry(Task, node, queue_pop(&sema->waiters));
        task_unblock(task);
    }
    sema->value++;
    assert(sema->value == 1);
    set_interrupt_status(old);
}

void acquire(Lock *lock)
{
    if (lock->holder != running_task())
    {
        // DEBUGP("holder 0x%08X task 0x%08X\n", lock->holder, running_task());
        sema_down(&lock->sema);
        // DEBUGP("holder 0x%08X task 0x%08X reapeat %d\n", lock->holder, running_task(), lock->repeat);
        lock->holder = running_task();
        assert(lock->repeat == 0);
        lock->repeat = 1;
    }
    else
    {
        DEBUGP("reenter acquire...\n");
        lock->repeat++;
    }
}

void release(Lock *lock)
{
    assert(lock->holder == running_task());
    if (lock->repeat > 1)
    {
        lock->repeat--;
        return;
    }
    assert(lock->repeat == 1);
    lock->holder = NULL;
    lock->repeat = 0;
    sema_up(&lock->sema);
}