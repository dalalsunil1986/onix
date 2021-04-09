#include <fs/onix/fs.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/string.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

u32 onix_block_lba(Partition *part, u32 idx)
{
    return part->super_block->data_start_lba + idx * BLOCK_SECTOR_COUNT;
}

void onix_block_lba_read(Partition *part, u32 lba, void *buf)
{
    partition_read(part, lba, buf, BLOCK_SECTOR_COUNT);
}

void onix_block_lba_write(Partition *part, u32 lba, void *buf)
{
    partition_write(part, lba, buf, BLOCK_SECTOR_COUNT);
}

void onix_block_read(Partition *part, u32 idx, void *buf)
{
    u32 lba = onix_block_lba(part, idx);
    return onix_block_lba_read(part, lba, buf);
}

void onix_block_write(Partition *part, u32 idx, void *buf)
{
    u32 lba = onix_block_lba(part, idx);
    return onix_block_lba_write(part, lba, buf);
}

void onix_block_loads(Partition *part, Inode *inode, u32 blocks[INODE_ALL_BLOCKS])
{
    memset(blocks, 0, ALL_BLOCKS_SIZE);
    memcpy(blocks, inode->blocks, DIRECT_BLOCK_CNT * sizeof(u32));
    u32 idx = inode->blocks[INDIRECT_BLOCK_IDX];
    if (idx)
    {
        onix_block_read(part, idx, blocks + INDIRECT_BLOCK_IDX);
    }
}