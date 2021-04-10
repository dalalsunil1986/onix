#ifndef ONIX_FSBLOCK_H
#define ONIX_FSBLOCK_H

#include <onix/types.h>
#include <onix/kernel/harddisk.h>

u32 onix_block_lba(Partition *part, u32 idx);

void onix_block_lba_read(Partition *part, u32 lba, void *buf);
void onix_block_lba_write(Partition *part, u32 lba, void *buf);

void onix_block_read(Partition *part, u32 idx, void *buf);
void onix_block_write(Partition *part, u32 idx, void *buf);

u32 onix_block_loads(Partition *part, Inode *inode, u32 blocks[INODE_ALL_BLOCKS]);

void onix_block_sync_indirect(Partition *part, u32 indirect_idx, u32 blocks[INODE_ALL_BLOCKS]);

#endif
