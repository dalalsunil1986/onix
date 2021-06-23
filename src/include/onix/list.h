/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#ifndef ONIX_LIST_H
#define ONIX_LIST_H

#include <onix/types.h>

#define element_offset(type, member) (u32)(&((type *)0)->member)
#define element_entry(type, member, ptr) (type *)((u32)ptr - element_offset(type, member))

typedef struct list_node_t
{
    struct list_node_t *prev;
    struct list_node_t *next;
} list_node_t;

typedef struct list_t
{
    list_node_t head;
    list_node_t tail;
} list_t;

typedef bool(traversal)(list_node_t *, int arg);

void list_init(list_t *list);

void list_push(list_t *list, list_node_t *node);
void list_pushback(list_t *list, list_node_t *node);
list_node_t *list_pop(list_t *list);
list_node_t *list_popback(list_t *list);

bool list_find(list_t *list, list_node_t *node);
bool list_remove(list_node_t *node);

bool list_empty(list_t *list);
bool list_size(list_t *list);

list_node_t *list_traversal(list_t *list, traversal visit, int arg);

#endif