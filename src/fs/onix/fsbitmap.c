#include <fs/onix/fsbitmap.h>
#include <onix/bitmap.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

int32 onix_inode_bitmap_alloc(Partition *part)
{
    int32 idx = bitmap_scan(&part->inode_bitmap, 1);
    if (idx == -1)
        return -1;
    bitmap_set(&part->inode_bitmap, idx, 1);
    return idx;
}

int32 onix_block_bitmap_alloc(Partition *part)
{
    int32 idx = bitmap_scan(&part->block_bitmap, 1);
    if (idx == -1)
        return -1;
    bitmap_set(&part->block_bitmap, idx, 1);
    return idx;
}

bool onix_bitmap_sync(Partition *part, u32 idx, BitmapType type)
{
    u32 offset_sects = idx / SECTOR_SIZE / 8;
    u32 offset_size = offset_sects * SECTOR_SIZE;
    u32 sec_lba;

    u8 *bitmap_offset;

    switch (type)
    {
    case INODE_BITMAP:
        sec_lba = part->super_block->inode_bitmap_lba + offset_sects;
        bitmap_offset = part->inode_bitmap.bits + offset_size;
        break;
    case BLOCK_BITMAP:
        sec_lba = part->super_block->block_bitmap_lba + offset_sects;
        bitmap_offset = part->block_bitmap.bits + offset_size;
        break;
    default:
        break;
    }
    partition_write(part, sec_lba, bitmap_offset, 1);
    return true;
}

int32 onix_inode_bitmap_alloc_sync(Partition *part)
{
    u32 idx = onix_inode_bitmap_alloc(part);
    if (idx == -1)
        return -1;
    onix_bitmap_sync(part, idx, INODE_BITMAP);
    return idx;
}

int32 onix_block_bitmap_alloc_sync(Partition *part)
{
    u32 idx = onix_block_bitmap_alloc(part);
    if (idx == -1)
        return -1;
    onix_bitmap_sync(part, idx, BLOCK_BITMAP);
    return idx;
}

void onix_inode_bitmap_rollback(Partition *part, u32 idx)
{
    assert(idx < part->inode_bitmap.length * 8);
    bitmap_set(&part->inode_bitmap, idx, 0);
}

void onix_block_bitmap_rollback(Partition *part, u32 idx)
{
    assert(idx < part->block_bitmap.length * 8);
    bitmap_set(&part->block_bitmap, idx, 0);
}

void onix_inode_bitmap_rollback_sync(Partition *part, u32 idx)
{
    onix_inode_bitmap_rollback(part, idx);
    onix_bitmap_sync(part, idx, INODE_BITMAP);
}

void onix_block_bitmap_rollback_sync(Partition *part, u32 idx)
{
    onix_block_bitmap_rollback(part, idx);
    onix_bitmap_sync(part, idx, BLOCK_BITMAP);
}