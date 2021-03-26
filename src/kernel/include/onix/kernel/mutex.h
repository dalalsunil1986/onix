#ifndef ONIX_MUTEX_H
#define ONIX_MUTEX_H

#include <onix/types.h>
#include <onix/queue.h>
#include <onix/kernel/task.h>

typedef struct Semaphore
{
    bool value;
    Queue waiters;
} Semaphore;

typedef struct Lock
{
    Task *holder;
    Semaphore sema;
    u32 repeat;
} Lock;

void lock_init(Lock *lock);
void acquire(Lock *lock);
void release(Lock *lock);

#endif