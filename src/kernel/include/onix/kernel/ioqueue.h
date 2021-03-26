#ifndef ONIX_IOQUEUE_H
#define ONIX_IOQUEUE_H

#include <onix/types.h>
#include <onix/kernel/mutex.h>

#define BUFFER_SIZE 16

typedef struct IOQueue
{
    Lock lock;
    Task *producer;
    Task *consumer;
    char buffer[BUFFER_SIZE];
    u32 head;
    u32 tail;
} IOQueue;

void ioqueue_init(IOQueue *ioq);
bool ioqueue_full(IOQueue *ioq);
bool ioqueue_empty(IOQueue *ioq);
char ioqueue_get(IOQueue *ioq);
void ioqueue_put(IOQueue *ioq, char byte);

#endif