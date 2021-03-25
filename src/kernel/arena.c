#include <onix/kernel/arena.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/memory.h>
#include <onix/string.h>
#include <onix/stdlib.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

ArenaDesc arena_descs[DESC_COUNT];

static void init_arena_desc()
{
    u32 block_size = 16;
    DEBUGP("Initializing arena desc...\n");
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        ArenaDesc *desc = arena_descs + i;
        desc->block_size = block_size;
        desc->total_block = (PG_SIZE - sizeof(Arena)) / block_size;
        queue_init(&desc->free_list);
        DEBUGP("init arena desc bsize %d total %d\n",
               block_size, desc->total_block);
        block_size <<= 1; // block *= 2;
    }
}

static void *get_arena_block(Arena *arena, u32 idx)
{
    assert(arena->desc->total_block > idx);
    void *addr = (void *)(arena + 1);
    u32 gap = idx * arena->desc->block_size;
    return addr + gap;
}

static Arena *get_block_arena(Block *block)
{
    return (Arena *)((u32)block & 0xfffff000);
}

void *malloc(size_t size)
{
    if (size < 0 || size > free_pages * PG_SIZE)
    {
        return NULL;
    }

    ArenaDesc *desc = NULL;
    Arena *arena;
    Block *block;
    void *addr;

    // TODO require lock

    if (size > 1024)
    {
        size += sizeof(Arena);
        size = round_up(size, PG_SIZE);
        u32 page_count = size / PG_SIZE;
        DEBUGP("get page size %d page count %d\n", size, page_count);
        arena = page_alloc(USER_KERNEL, page_count);
        memset(arena, 0, size);
        arena->large = true;
        arena->count = page_count;
        arena->desc = NULL;
        return (void *)(arena + 1);
    }
    else
    {
        for (size_t i = 0; i < DESC_COUNT; i++)
        {
            desc = arena_descs + i;
            if (desc->block_size >= size)
                break;
            /* code */
        }

        assert(desc != NULL);
        if (queue_empty(&desc->free_list))
        {
            arena = page_alloc(USER_KERNEL, 1);
            memset(arena, 0, PG_SIZE);
            arena->desc = desc;
            arena->large = false;
            arena->count = desc->total_block;

            for (size_t i = 0; i < desc->total_block; i++)
            {
                block = get_arena_block(arena, i);
                assert(!queue_find(&arena->desc->free_list, block));
                DEBUGP("push block 0x%X size %d qsize %d\n",
                       block, desc->block_size, desc->free_list.size);
                queue_push(&arena->desc->free_list, block);
                assert(queue_find(&arena->desc->free_list, block));
            }
        }
        block = (Block *)queue_pop(&desc->free_list);
        memset(block, 0, desc->block_size);
        arena = get_block_arena(block);
        arena->count--;
        addr = (void *)block;
    }

    // TODO release lock
    return addr;
}

void free(void *ptr)
{
    assert(ptr != NULL);
    Block *block = ptr;
    Arena *arena = get_block_arena(block);
    assert(arena->large == 1 || arena->large == 0);
    if (arena->large == true)
    {
        page_free(USER_KERNEL, arena, arena->count);
        return;
    }
    queue_push(&arena->desc->free_list, block);

    if (arena->desc->free_list.size == arena->desc->total_block)
    {
        for (size_t i = 0; i < arena->desc->total_block; i++)
        {
            block = get_arena_block(arena, i);
            DEBUGP("remove block 0x%X size %d qsize %d\n", block,
                   arena->desc->block_size, arena->desc->free_list.size);
            assert(queue_find(&arena->desc->free_list, block));
            queue_remove(block);
            assert(!queue_find(&arena->desc->free_list, block));
        }
        page_free(USER_KERNEL, arena, 1);
        return;
    }
}

static void test_arena()
{
    DEBUGK("Test arena.....\n");
    int size = 4096;
    DEBUGP("free pages %d\n", free_pages);
    u32 *test = malloc(size);

    DEBUGP("get test memory 0x%X free pages %d\n", test, free_pages);
    // BMB;
    for (size_t i = 0; i < size / 4; i++)
    {
        DEBUGP("set test memory %i\n", i);
        test[i] = i;
    }
    free(test);
    DEBUGP("free pages %d\n", free_pages);
}

void init_arena()
{
    DEBUGP("Initializing arena...\n");
    init_arena_desc();
    test_arena();
}