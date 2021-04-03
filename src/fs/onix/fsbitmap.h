#ifndef ONIX_FSBITMAP_H
#define ONIX_FSBITMAP_H

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

int32 inode_bitmap_alloc(Partition *part);
int32 block_bitmap_alloc(Partition *part);

void inode_bitmap_rollback(Partition *part, u32 idx);
void block_bitmap_rollback(Partition *part, u32 idx);

bool bitmap_sync(Partition *part, u32 idx, BitmapType type);

int32 inode_bitmap_alloc_sync(Partition *part);
int32 block_bitmap_alloc_sync(Partition *part);

void inode_bitmap_rollback_sync(Partition *part, u32 idx);
void block_bitmap_rollback_sync(Partition *part, u32 idx);

#endif