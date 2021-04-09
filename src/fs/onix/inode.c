#include <fs/onix/inode.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/assert.h>
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

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return;
    }
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

static Inode *onix_inode_search(Partition *part, u32 nr)
{
    Node *node = part->open_inodes.head.next;
    Inode *inode;

    while (node != &part->open_inodes.tail)
    {
        inode = element_entry(Inode, node, node);
        if (inode->nr == nr)
        {
            return inode;
        }
        node = node->next;
    }
    return NULL;
}

Inode *onix_inode_open(Partition *part, u32 nr)
{
    Inode *inode = onix_inode_search(part, nr);
    if (inode != NULL)
    {
        inode->open_cnts++;
        return inode;
    }

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return NULL;
    }

    InodePosition pos;
    onix_inode_locate(part, nr, &pos);

    inode = malloc(sizeof(Inode));

    onix_block_lba_read(part, pos.sec_lba, buf);

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
    assert(queue_find(&part->open_inodes, &inode->node));
    inode->open_cnts--;
    if (inode->open_cnts == 0)
    {
        queue_remove(&part->open_inodes, &inode->node);
        free(inode);
    }
    set_interrupt_status(old);
}

void onix_inode_erase(Partition *part, u32 nr)
{
    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return;
    }

    InodePosition pos;
    onix_inode_locate(part, nr, &pos);
    onix_block_lba_read(part, pos.sec_lba, buf);
    memset(buf + pos.offset, 0, sizeof(Inode));
    onix_block_lba_write(part, pos.sec_lba, buf);
    free(buf);
}

void onix_inode_delete(Partition *part, u32 nr)
{
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        return;
    }

    Inode *inode = onix_inode_open(part, nr);
    assert(inode->nr == nr);

    onix_block_loads(part, inode, blocks);

    u32 idx = inode->blocks[INDIRECT_BLOCK_IDX];
    if (idx)
    {
        onix_block_bitmap_rollback_sync(part, idx);
    }

    idx = 0;
    while (blocks[idx])
    {
        onix_block_bitmap_rollback_sync(part, blocks[idx]);
        idx++;
    }
    onix_inode_bitmap_rollback_sync(part, nr);
    onix_inode_close(part, inode);
    free(blocks);
}

void onix_inode_init(u32 nr, Inode *inode)
{
    inode->nr = nr;
    inode->size = 0;
    inode->open_cnts = 0;
    inode->write_deny = false;
    memset(inode->blocks, 0, sizeof(inode->blocks));
    memset(inode->unused, 0, sizeof(inode->unused));
}