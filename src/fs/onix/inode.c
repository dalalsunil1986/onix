#include <onix/inode.h>
#include <onix/kernel/harddisk.h>

static void inode_locate(Partition *part, u32 nr, InodePosition *pos)
{
    assert(nr < MAX_FILE_PER_PART);

    u32 inode_table_lba = part->super_block->inode_table_lba;

    u32 inode_size = sizeof(Inode);
    u32 offset_size = nr * inode_size;
    u32 offset_blocks = offset_size / BLOCK_SIZE;
    u32 offset_in_block = offset_size % BLOCK_SIZE;
    u32 left_in_block = BLOCK_SIZE - offset_in_block;

    pos->cross = false;
    if (left_in_block < inode_size)
    {
        pos->cross = true;
    }
    pos->sec_lba = inode_table_lba + offset_blocks;
    pos->offset = offset_in_block;
}

void inode_sync(Partition *part, Inode *inode, void *buf)
{
    u32 nr = inode->nr;
    InodePosition pos;
    inode_locate(part, nr, &pos);

    assert(pos.sec_lba <= (part->start_lba + part->sec_cnt));

    Inode pure_inode;

    memcpy(&pure_inode, inode, sizeof(Inode));

    pure_inode.open_cnts = 0;
    pure_inode.write_deny = false;
    pure_inode.node.prev = NULL;
    pure_inode.node.next = NULL;

    u8 block_count = 1;
    if (pos.cross)
    {
        block_count = 2;
    }

    harddisk_read(part->disk, pos.sec_lba, buf, block_count * BLOCK_SECTOR_COUNT);
    memcpy((buf + pos.offset), &pure_inode, sizeof(Inode));
    harddisk_write(part->disk, pos.sec_lba, buf, block_count * BLOCK_SECTOR_COUNT);
}

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