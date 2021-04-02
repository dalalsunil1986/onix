#ifndef ONIX_FSBITMAP_H
#define ONIX_FSBITMAP_H

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

int32 inode_bitmap_alloc(Partition *part);
int32 block_bitmap_alloc(Partition *part);

#endif