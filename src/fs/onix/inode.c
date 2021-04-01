#include <onix/inode.h>

void inode_init(u32 nr, Inode *inode)
{
    inode->nr = nr;
    inode->size = 0;
    inode->open_cnts = 0;
    inode->write_deny = false;

    u8 idx = 0;
    while (idx < INODE_BLOCK_CNT)
    {
        inode->blocks[idx] = 0;
        idx++;
    }
}