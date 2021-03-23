#include <onix/queue.h>
#include <onix/kernel/memory.h>

void queue_init(Queue *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
}

void queue_push(Queue *queue, Node *node)
{
    if (queue->head == NULL)
    {
        queue->head = node;
        queue->tail = node;
    }
    queue->tail->next = node;
    queue->tail = node;
}

Node *queue_pop(Queue *queue)
{
    if (queue->head == NULL)
        return NULL;
    Node *node = queue->head;
    queue->head = queue->head->next;
    return node;
}

bool queue_empty(Queue *queue)
{
    return (queue->head == NULL);
}