#include <fs/onix/fs.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

u32 get_block_lba(Partition *part, u32 idx)
{
    return part->super_block->data_start_lba + idx * BLOCK_SECTOR_COUNT;
}