#include <onix/queue.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/debug.h>

#define DEBUGP DEBUGK
// #define DEBUGP(fmt, args...)

void queue_init(Queue *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

void queue_push(Queue *queue, Node *node)
{
    queue->size++;
    DEBUGP("PUSH queue 0x%X node 0x%X size %d\n", queue, node, queue->size);

    if (queue->head == NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        queue->head = node;
        queue->tail = node;
        return;
    }
    node->prev = queue->tail;
    queue->tail->next = node;
    queue->tail = node;
}

void queue_pushback(Queue *queue, Node *node)
{
    DEBUGP("PUSHBACK queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    queue->size++;
    if (queue->head == NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        queue->head = node;
        queue->tail = node;
        return;
    }
    node->next = queue->head;
    queue->head->prev = node;
    queue->head = node;
}

Node *queue_pop(Queue *queue)
{
    if (queue->head == NULL)
        return NULL;

    assert(queue->size > 0);

    queue->size--;
    Node *node = queue->head;
    queue->head = queue->head->next;
    if (queue->head)
    {
        queue->head->prev = NULL;
    }
    DEBUGP("POP queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    return node;
}

Node *queue_popback(Queue *queue)
{
    if (queue->tail == NULL)
        return NULL;

    assert(queue->size > 0);

    queue->size--;
    Node *node = queue->tail;
    queue->tail = queue->tail->prev;
    if (queue->tail)
    {
        queue->tail->next = NULL;
    }
    DEBUGP("POPBACK queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    return node;
}

bool queue_empty(Queue *queue)
{
    return (queue->head == NULL);
}

bool queue_find(Queue *queue, Node *node)
{
    Node *next = queue->head;
    while (next != NULL)
    {
        DEBUGP("Found next 0x%X node 0x%x\n", next, node);
        if (next == node)
            return true;
        next = next->next;
    }
    return false;
}