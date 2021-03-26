#include <onix/kernel/ioqueue.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

void ioqueue_init(IOQueue *ioq)
{
    lock_init(&ioq->lock);
    ioq->producer = NULL;
    ioq->consumer = NULL;
    ioq->head = 0;
    ioq->tail = 0;
}

static u32 ioqueue_next_pos(u32 pos)
{
    return (pos + 1) % BUFFER_SIZE;
}

bool ioqueue_full(IOQueue *ioq)
{
    assert(!get_interrupt_status()); // 禁止中断
    bool full = (ioqueue_next_pos(ioq->head) == ioq->tail);
    // DEBUGP("ioqueue full %d\n", full);
    return full;
}

bool ioqueue_empty(IOQueue *ioq)
{
    assert(!get_interrupt_status()); // 禁止中断
    return ioq->head == ioq->tail;
}

static void ioqueue_wait(Task **waiter)
{
    // DEBUGP("iowaiter 0x%08X waiter 0x%08X\n", waiter, *waiter);
    assert((waiter != NULL) && (*waiter == NULL));
    *waiter = running_task();
    task_block(*waiter);
}

static void ioqueue_weakup(Task **waiter)
{
    assert(*waiter != NULL);
    task_unblock(*waiter);
    *waiter = NULL;
}

char ioqueue_get(IOQueue *ioq)
{
    assert(!get_interrupt_status());

    while (ioqueue_empty(ioq))
    {
        acquire(&ioq->lock);
        ioqueue_wait(&ioq->consumer);
        release(&ioq->lock);
    }
    char byte = ioq->buffer[ioq->tail];
    ioq->tail = ioqueue_next_pos(ioq->tail);

    if (ioq->producer != NULL)
    {
        ioqueue_weakup(&ioq->producer);
    }
    // DEBUGP("%d\n", byte);
    return byte;
}

void ioqueue_put(IOQueue *ioq, char byte)
{
    assert(!get_interrupt_status());
    while (ioqueue_full(ioq))
    {
        // DEBUGP("wait full....\n");
        acquire(&ioq->lock);
        ioqueue_wait(&ioq->producer);
        release(&ioq->lock);
    }
    ioq->buffer[ioq->head] = byte;
    ioq->head = ioqueue_next_pos(ioq->head);

    if (ioq->consumer != NULL)
    {
        ioqueue_weakup(&ioq->consumer);
    }
}
