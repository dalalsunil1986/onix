#include <onix/queue.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/debug.h>

// #define DEBUGP DEBUGK
#define DEBUGP(fmt, args...)

void queue_init(Queue *queue)
{
    queue->head.prev = NULL;
    queue->tail.next = NULL;
    queue->head.next = &queue->tail;
    queue->tail.prev = &queue->head;
    queue->size = 0;
}

void queue_push(Queue *queue, Node *node)
{
    DEBUGP("PUSH queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    node->prev = &queue->head;
    node->next = queue->head.next;
    queue->head.next->prev = node;
    queue->head.next = node;
    queue->size++;
}

Node *queue_pop(Queue *queue)
{
    assert(queue->size > 0);

    Node *node = queue->head.next;
    node->next->prev = &queue->head;
    queue->head.next = node->next;
    queue->size--;
    DEBUGP("POP queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    return node;
}

void queue_pushback(Queue *queue, Node *node)
{
    DEBUGP("PUSHBACK queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    node->next = &queue->tail;
    node->prev = queue->tail.prev;
    queue->tail.prev->next = node;
    queue->tail.prev = node;
    queue->size++;
}

Node *queue_popback(Queue *queue)
{
    assert(queue->size > 0);
    Node *node = queue->tail.prev;
    node->prev->next = &queue->tail;
    queue->tail.prev = node->prev;
    queue->size--;
    DEBUGP("POPBACK queue 0x%X node 0x%X size %d\n", queue, node, queue->size);
    return node;
}

bool queue_empty(Queue *queue)
{
    return (queue->size == 0);
}

bool queue_find(Queue *queue, Node *node)
{
    Node *next = queue->head.next;
    while (next != &queue->tail)
    {
        DEBUGP("Found next 0x%X node 0x%x\n", next, node);
        if (next == node)
            return true;
        next = next->next;
    }
    return false;
}

bool queue_remove(Queue *queue, Node *node)
{
    DEBUGP("node 0x%X prev 0x%X next 0x%X\n", node, node->prev, node->next);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    queue->size--;
}

Node *queue_traversal(Queue *queue, traversal visit, int arg)
{
    if (queue_empty(queue))
        return NULL;

    Node *node = queue->head.next;
    while (node != &queue->tail)
    {
        if (visit(node, arg))
        {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void test_queue()
{
    Queue q;
    Node node_a;
    Node node_b;

    Queue *queue = &q;
    Node *a = &node_a;
    Node *b = &node_b;

    queue_init(queue);
    assert(queue->size == 0);
    queue_push(queue, a);
    assert(queue->size == 1);
    assert(queue_find(queue, a));
    assert(queue_pop(queue) == a);
    assert(!queue_find(queue, a));
    assert(queue->size == 0);
    queue_pushback(queue, a);
    queue_pushback(queue, b);
    assert(queue_pop(queue) == a);
    assert(queue_pop(queue) == b);

    queue_push(queue, a);
    queue_push(queue, b);
    assert(queue_popback(queue) == a);
    assert(queue_popback(queue) == b);

    queue_push(queue, a);
    queue_push(queue, b);
    assert(queue_find(queue, a));
    assert(queue_find(queue, b));
    queue_remove(queue, a);
    assert(!queue_find(queue, a));
    assert(queue_find(queue, b));
    // BMB;
}