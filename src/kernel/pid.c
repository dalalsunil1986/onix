#include <onix/kernel/pid.h>
#include <onix/kernel/mutex.h>
#include <onix/assert.h>
#include <onix/bitmap.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

#define BITS_LEN ((MAX_PID + 1) / 8)

u8 bits[BITS_LEN];

typedef struct PIDPool
{
    Bitmap map;
    u32 start;
    Lock lock;
} PIDPool;

static PIDPool pool;

void init_pid()
{
    DEBUGK("init pid...\n");
    pool.map.bits = bits;
    pool.map.length = BITS_LEN;
    pool.start = 0;
    bitmap_init(&pool.map);
    lock_init(&pool.lock);
}

pid_t allocate_pid()
{
    Bitmap *map = &pool.map;
    acquire(&pool.lock);
    u32 idx = bitmap_scan(map, 1);
    assert(idx != -1);
    bitmap_set(map, idx, 1);
    release(&pool.lock);
    return pool.start + idx;
}

void release_pid(pid_t pid)
{
    Bitmap *map = &pool.map;
    acquire(&pool.lock);
    u32 idx = pid - pool.start;
    assert(bitmap_test(map, idx));
    bitmap_set(map, idx, 0);
    release(&pool.lock);
}

bool test_pid(pid_t pid)
{
    Bitmap *map = &pool.map;
    acquire(&pool.lock);
    u32 idx = pid - pool.start;
    bool res = bitmap_test(map, idx);
    release(&pool.lock);
    return res;
}