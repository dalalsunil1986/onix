#ifndef ONIX_FSBITMAP_H
#define ONIX_FSBITMAP_H

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

#define ONIX_INODE_FAILURE ({printk("Onix inode allocate fail!!!\n"); return -1; })
#define ONIX_BLOCK_FAILURE ({printk("Onix block allocate fail!!!\n"); return -1; })

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