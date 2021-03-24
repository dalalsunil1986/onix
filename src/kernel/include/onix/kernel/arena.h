#ifndef ONIX_ARENA_H
#define ONIX_ARENA_H

#include <onix/types.h>
#include <onix/queue.h>

#define DESC_COUNT 7

typedef struct Block
{
    Node block;
} Block;

typedef struct BlockDesc
{
    u32 total_size;
    u32 block_size;
    Queue free_list;
} BlockDesc;

typedef struct Arena
{
    BlockDesc *desc;
    u32 count;
    bool large;
} Arena;

void init_arena();

#endif