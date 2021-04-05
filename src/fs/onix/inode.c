#include <fs/onix/inode.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/assert.h>
#include <onix/string.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

static void onix_inode_locate(Partition *part, u32 nr, InodePosition *pos)
{
    assert(nr < MAX_FILE_PER_PART);

    u32 inode_table_lba = part->super_block->inode_table_lba;

    u32 inode_size = sizeof(Inode);

    u32 offset_size = nr * inode_size;
    u32 offset_blocks = offset_size / BLOCK_SIZE;
    u32 offset_in_block = offset_size % BLOCK_SIZE;

    pos->sec_lba = inode_table_lba + offset_blocks * BLOCK_SECTOR_COUNT;
    pos->offset = offset_in_block;
}

void onix_inode_sync(Partition *part, Inode *inode)
{
    u32 nr = inode->nr;
    InodePosition pos;
    onix_inode_locate(part, nr, &pos);

    char *buf = malloc(BLOCK_SIZE); // todo rollback;
    memset(buf, 0, BLOCK_SIZE);

    assert(pos.sec_lba <= part->sec_cnt);

    Inode pure_inode;

    memcpy(&pure_inode, inode, sizeof(Inode));

    pure_inode.open_cnts = 0;
    pure_inode.write_deny = false;
    pure_inode.node.prev = NULL;
    pure_inode.node.next = NULL;

    partition_read(part, pos.sec_lba, buf, BLOCK_SECTOR_COUNT);
    memcpy((buf + pos.offset), &pure_inode, sizeof(Inode));
    partition_write(part, pos.sec_lba, buf, BLOCK_SECTOR_COUNT);
    free(buf);
}

Inode *onix_inode_open(Partition *part, u32 nr)
{
    Node *node = part->open_inodes.head.next;
    Inode *inode;

    while (node != &part->open_inodes.tail)
    {
        inode = element_entry(Inode, node, node);
        if (inode->nr == nr)
        {
            inode->open_cnts++;
            return inode;
        }
        node = node->next;
    }

    InodePosition pos;
    onix_inode_locate(part, nr, &pos);

    inode = malloc(sizeof(Inode));

    char *buf = malloc(BLOCK_SIZE);
    // todo rollback....

    partition_read(part, pos.sec_lba, buf, BLOCK_SECTOR_COUNT);

    memcpy(inode, buf + pos.offset, sizeof(Inode));

    queue_push(&part->open_inodes, &inode->node);
    inode->open_cnts = 1;
    inode->nr = nr;

    free(buf);
    return inode;
}

void onix_inode_close(Partition *part, Inode *inode)
{
    bool old = disable_int();
    if (--(inode->open_cnts) == 0)
    {
        queue_remove(&part->open_inodes, &inode->node);
        free(inode);
    }
    set_interrupt_status(old);
}

void onix_inode_erase(Partition *part, u32 nr)
{
    InodePosition pos;
    onix_inode_locate(part, nr, &pos);
    char buf[BLOCK_SIZE];
    partition_read(part, pos.sec_lba, buf, BLOCK_SECTOR_COUNT);
    memset(buf + pos.offset, 0, sizeof(Inode));
    partition_write(part, pos.sec_lba, buf, BLOCK_SECTOR_COUNT);
}

void onix_inode_delete(Partition *part, u32 nr)
{
    Inode *inode = onix_inode_open(part, nr);
    assert(inode->nr == nr);

    u32 blocks[INODE_ALL_BLOCKS];
    memcpy(blocks, inode->blocks, DIRECT_BLOCK_CNT * sizeof(u32));

    u32 lba = 0;
    u32 indirect_idx = inode->blocks[INDIRECT_BLOCK_IDX];
    if (indirect_idx)
    {
        lba = get_block_lba(part, indirect_idx);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
        assert(indirect_idx > 0);
        onix_block_bitmap_rollback_sync(part, indirect_idx);
    }
    u32 idx = 0;
    while (blocks[idx])
    {
        onix_block_bitmap_rollback_sync(part, blocks[idx]);
        idx++;
    }
    onix_inode_bitmap_rollback_sync(part, nr);
    onix_inode_close(part, inode);
}

void onix_inode_init(u32 nr, Inode *inode)
{
    inode->nr = nr;
    inode->size = 0;
    inode->open_cnts = 0;
    inode->write_deny = false;

    memset(inode->blocks, 0, sizeof(inode->blocks));

    // fuck，内存没有初始化
    // 还以为我哪儿写错了，调试了半天
    memset(inode->unused, 0, sizeof(inode->unused));
}