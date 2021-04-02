#include <fs/onix/inode.h>
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
    pos->sec_lba = inode_table_lba + offset_blocks * BLOCK_SECTOR_COUNT;
    pos->offset = offset_in_block;
}

void inode_sync(Partition *part, Inode *inode)
{
    u32 nr = inode->nr;
    InodePosition pos;
    inode_locate(part, nr, &pos);

    char *buf[BLOCK_SIZE * BLOCK_SECTOR_COUNT];
    memset(buf, 0, BLOCK_SIZE * BLOCK_SECTOR_COUNT);

    assert(pos.sec_lba <= part->sec_cnt);

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

    partition_read(part, pos.sec_lba, buf, block_count * BLOCK_SECTOR_COUNT);
    memcpy((buf + pos.offset), &pure_inode, sizeof(Inode));
    partition_write(part, pos.sec_lba, buf, block_count * BLOCK_SECTOR_COUNT);
    DEBUGP("part 0x%X lba line %d\n", part, (part->start_lba + pos.sec_lba) * SECTOR_SIZE / 16);
}

Inode *inode_open(Partition *part, u32 nr)
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
    inode_locate(part, nr, &pos);

    inode = malloc(sizeof(Inode));

    int block_count = 1;
    if (pos.cross)
        block_count = 2;

    char *buf = malloc(BLOCK_SIZE * block_count);
    partition_read(part, pos.sec_lba, buf, block_count * BLOCK_SECTOR_COUNT);

    memcpy(inode, buf + pos.offset, sizeof(Inode));

    queue_push(&part->open_inodes, &inode->node);
    inode->open_cnts = 1;
    inode->nr = nr;

    free(buf);
    return inode;
}

void inode_close(Partition *part, Inode *inode)
{
    bool old = disable_int();
    if (--(inode->open_cnts) == 0)
    {
        queue_remove(&part->open_inodes, &inode->node);
        free(inode);
    }
    set_interrupt_status(old);
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