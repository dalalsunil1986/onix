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
    u32 size;
} Queue;

void queue_init(Queue *queue);
bool queue_find(Queue *queue, Node *node);
void queue_push(Queue *queue, Node *node);
void queue_pushback(Queue *queue, Node *node);
Node *queue_pop(Queue *queue);
Node *queue_popback(Queue *queue);
bool queue_empty(Queue *queue);

#endif