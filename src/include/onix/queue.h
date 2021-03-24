#ifndef ONIX_LIST_H
#define ONIX_LIST_H

#include <onix/types.h>

#define element_offset(type, member) (u32)(&((type *)0)->member)
#define element_entry(type, member, ptr) (type *)((u32)ptr - element_offset(type, member))

typedef struct Node
{
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node head;
    Node tail;
    u32 size;
} Queue;

void queue_init(Queue *queue);
bool queue_find(Queue *queue, Node *node);
bool queue_remove(Node *node);
void queue_push(Queue *queue, Node *node);
void queue_pushback(Queue *queue, Node *node);
Node *queue_pop(Queue *queue);
Node *queue_popback(Queue *queue);
bool queue_empty(Queue *queue);

void test_queue();

#endif