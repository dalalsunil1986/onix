#ifndef ONIX_INODE_H
#define ONIX_INODE_H

#include <onix/types.h>
#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

typedef struct InodePosition
{
    u32 sec_lba;
    u32 offset;
} InodePosition;

void onix_inode_init(u32 nr, Inode *inode);
Inode *onix_inode_open(Partition *part, u32 nr);
void onix_inode_close(Partition *part, Inode *inode);
void onix_inode_sync(Partition *part, Inode *inode);
void onix_inode_erase(Partition *part, u32 nr);
void onix_inode_delete(Partition *part, u32 nr);

#endif