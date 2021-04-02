#ifndef ONIX_INODE_H
#define ONIX_INODE_H

#include <onix/types.h>
#include <fs/onix/fs.h>

typedef struct InodePosition
{
    bool cross;
    u32 sec_lba;
    u32 offset;
} InodePosition;

void inode_init(u32 nr, Inode *inode);
Inode *inode_open(Partition *part, u32 nr);
void inode_close(Partition *part, Inode *inode);
void inode_sync(Partition *part, Inode *inode);

#endif