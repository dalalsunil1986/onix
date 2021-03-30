#ifndef ONIX_ARENA_H
#define ONIX_ARENA_H

#include <onix/types.h>
#include <onix/queue.h>

#define DESC_COUNT 7

typedef struct Block
{
    struct Block *prev;
    struct Block *next;
} Block;

typedef struct ArenaDesc
{
    u32 total_block;
    u32 block_size;
    Queue free_list;
} ArenaDesc;

typedef struct Arena
{
    ArenaDesc *desc;
    u32 count;
    bool large;
} Arena;

void init_arena();
void init_arena_desc(ArenaDesc *descs);
extern void *arena_malloc(size_t size);
extern void arena_free(void *ptr);

#endif