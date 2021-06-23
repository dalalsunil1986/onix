/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#include <onix/list.h>
#include <onix/assert.h>

void list_init(list_t *list)
{
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

void list_push(list_t *list, list_node_t *node)
{
    assert(!list_find(list, node));

    node->prev = &list->head;
    node->next = list->head.next;

    list->head.next->prev = node;
    list->head.next = node;
}

list_node_t *list_pop(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->head.next;
    node->next->prev = &list->head;
    list->head.next = node->next;

    return node;
}

void list_pushback(list_t *list, list_node_t *node)
{
    assert(!list_find(list, node));

    node->next = &list->tail;
    node->prev = list->tail.prev;

    list->tail.prev->next = node;
    list->tail.prev = node;
}

list_node_t *list_popback(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->tail.prev;

    node->prev->next = &list->tail;
    list->tail.prev = node->prev;

    return node;
}

bool list_find(list_t *list, list_node_t *node)
{
    list_node_t *next = list->head.next;
    while (next != &list->tail)
    {
        if (next == node)
            return true;
        next = next->next;
    }
    return false;
}
bool list_remove(list_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

bool list_empty(list_t *list)
{
    return (list->head.next == &list->tail);
}

bool list_size(list_t *list)
{
    list_node_t *next = list->head.next;
    u32 size = 0;
    while (next != &list->tail)
    {
        size++;
        next = next->next;
    }
    return size;
}

list_node_t *list_traversal(list_t *list, traversal visit, int arg)
{
    if (list_empty(list))
        return NULL;

    list_node_t *node = list->tail.prev;
    while (node != &list->head)
    {
        if (visit(node, arg))
        {
            return node;
        }
        node = node->prev;
    }
    return NULL;
}
