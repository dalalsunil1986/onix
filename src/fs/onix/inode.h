#ifndef ONIX_INODE_H
#define ONIX_INODE_H

#include <onix/types.h>
#include <onix/fs.h>

typedef struct InodePosition
{
    bool cross;
    u32 sec_lba;
    u32 offset;
} InodePosition;

void inode_init(u32 nr, Inode *inode);

#endif