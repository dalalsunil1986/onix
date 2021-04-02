#include <fs/onix/fs.h>
#include <fs/onix/fsblock.h>
#include <onix/kernel/harddisk.h>

u32 get_block_lba(Partition *part, u32 idx)
{
    return part->super_block->data_start_lba + idx * BLOCK_SECTOR_COUNT;
}