#include <fs/onix/fsbitmap.h>
#include <onix/bitmap.h>
#include <onix/kernel/harddisk.h>

int32 inode_bitmap_alloc(Partition *part)
{
    int32 idx = bitmap_scan(&part->inode_bitmap, 1);
    if (idx == -1)
        return -1;
    bitmap_set(&part->inode_bitmap, idx, 1);
    return idx;
}

int32 block_bitmap_alloc(Partition *part)
{
    int32 idx = bitmap_scan(&part->block_bitmap, 1);
    if (idx == -1)
        return -1;
    bitmap_set(&part->block_bitmap, idx, 1);
    return part->super_block->data_start_lba + idx;
}

u32 bitmap_sync(Partition *part, u32 idx, BitmapType type)
{
    u32 offset_block = idx / BLOCK_SIZE / 8;
    u32 offset_size = offset_block * BLOCK_SIZE;
    u32 sec_lba;
    u8 *bitmap_offset;

    switch (type)
    {
    case INODE_BITMAP:
        sec_lba = part->super_block->inode_bitmap_lba + offset_block;
        bitmap_offset = part->inode_bitmap.bits + offset_size;
        break;
    case BLOCK_BITMAP:
        sec_lba = part->super_block->block_bitmap_lba + offset_block;
        bitmap_offset = part->block_bitmap.bits + offset_size;
        break;
    default:
        break;
    }
    harddisk_write(part->disk, sec_lba, bitmap_offset, BLOCK_SECTOR_COUNT);
}