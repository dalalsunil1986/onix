#include <onix/kernel/arena.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/assert.h>

#define DEBUGP DEBUGK
// #define DEBUGP(fmt, args...)

BlockDesc block_desces[DESC_COUNT];

void init_arena()
{
    DEBUGP("Initializing arena...\n");
}
