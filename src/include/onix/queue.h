#ifndef ONIX_LIST_H
#define ONIX_LIST_H

#include <onix/types.h>

typedef struct Node
{
    struct Node *prev;
    struct Node *next;
    void *data;
} Node;

typedef struct Queue
{
    Node *head;
    Node *tail;
} Queue;

void queue_init(Queue *queue);
void queue_push(Queue *queue, Node *node);
Node *queue_pop(Queue *queue);
bool queue_empty(Queue *queue);

#endif