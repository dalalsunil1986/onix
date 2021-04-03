#include <fs/onix/fsdir.h>
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

Dir root_dir;
extern Partition *root_part;

Dir *open_root_dir(Partition *part)
{
    if (root_dir.inode != NULL)
    {
        return &root_dir;
    }
    root_dir.inode = onix_inode_open(part, part->super_block->root_inode_nr);
    root_dir.dir_offset = 0;
    return &root_dir;
}

Dir *dir_open(Partition *part, u32 nr)
{
    Dir *dir = malloc(sizeof(Dir));
    if (dir == NULL)
    {
        printk("dir open allocate memroy fail!!!\n");
        return NULL;
    }
    dir->inode = onix_inode_open(part, nr);
    dir->dir_offset = 0;
    return dir;
}

void dir_close(Partition *part, Dir *dir)
{
    if (dir == &root_dir)
        return;
    onix_inode_close(part, dir->inode);
    free(dir);
}

bool search_dir_entry(Partition *part, Dir *dir, char *name, DirEntry *entry)
{
    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32));
    if (blocks == NULL)
    {
        printk("allocate memory fail!!!\n");
        return false;
    }
    u8 *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        free(blocks);
        printk("allocate memory fail!!!\n");
        return false;
    }

    u32 idx = 0;
    while (idx < DIRECT_BLOCK_CNT)
    {
        blocks[idx] = dir->inode->blocks[idx];
        idx++;
    }
    if (dir->inode->blocks[idx] != 0)
    {
        partition_read(part, get_block_lba(part, idx), blocks + 12, BLOCK_SECTOR_COUNT);
    }

    DirEntry *dir_ptr = buf;
    u32 dir_entry_size = part->super_block->dir_entry_size;
    u32 dir_entry_cnt = BLOCK_SIZE / dir_entry_size;

    idx = 0;
    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx] == 0)
        {
            idx++;
            continue;
        }
        partition_read(part, get_block_lba(part, blocks[idx]), buf, BLOCK_SECTOR_COUNT);
        u32 entry_idx = 0;

        while (entry_idx < dir_entry_cnt)
        {
            if (!strcmp(dir_ptr->filename, name))
            {
                memcpy(entry, dir_ptr, dir_entry_size);
                free(buf);
                free(blocks);
                return true;
            }
            entry_idx++;
            dir_ptr++;
        }
        idx++;
        dir_ptr = buf;
        memset(buf, 0, SECTOR_SIZE);
    }
    free(buf);
    free(blocks);
    return false;
}

void create_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry)
{
    assert(strlen(filename) <= MAX_FILENAME_LENGTH);
    memset(entry->filename, 0, sizeof(entry->filename));
    memcpy(entry->filename, filename, strlen(filename));
    entry->inode_nr = nr;
    entry->type = type;
}

bool sync_dir_entry(Dir *parent, DirEntry *entry, void *buf)
{
    Inode *dir_inode = parent->inode;
    u32 dir_size = dir_inode->size;
    u32 dir_entry_size = root_part->super_block->dir_entry_size;

    assert(dir_size % dir_entry_size == 0);

    u32 dir_entry_cnt = (BLOCK_SIZE / dir_entry_size);
    int32 block_lba = -1;

    u32 idx = 0;
    u32 blocks[INODE_ALL_BLOCKS] = {0};

    while (idx < DIRECT_BLOCK_CNT)
    {
        blocks[idx] = dir_inode->blocks[idx];
        idx++;
    }

    DirEntry *dir_entry = buf;

    u32 lba = 0;
    int32 block_bitmap_idx = -1;
    Partition *part = root_part;

    idx = 0;
    bool flag = false;

    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx] != 0)
        {
            lba = get_block_lba(part, blocks[idx]);
            partition_read(part, lba, buf, BLOCK_SECTOR_COUNT);
            u8 dir_entry_idx = 0;
            while (dir_entry_idx < dir_entry_cnt)
            {
                DirEntry *dir_ptr = (dir_entry + dir_entry_idx);
                if (dir_ptr->type == FILETYPE_UNKNOWN)
                {
                    memcpy(dir_ptr, entry, dir_entry_size);
                    partition_write(part, lba, buf, BLOCK_SECTOR_COUNT);
                    dir_inode->size += dir_entry_size;
                    return true;
                }
                dir_entry_idx++;
            }
            idx++;
            continue;
        }

        block_bitmap_idx = onix_block_bitmap_alloc_sync(part);
        if (block_bitmap_idx == -1)
        {
            printk("allocate block bitmap failed ...\n");
            return false;
        }
        if (idx < DIRECT_BLOCK_CNT) // 直接块
        {
            dir_inode->blocks[idx] = blocks[idx] = block_bitmap_idx;
            continue;
        }
        else if (idx >= INDIRECT_BLOCK_IDX && flag) // 一级间接块
        {
            blocks[idx] = block_bitmap_idx;
            lba = get_block_lba(part, dir_inode->blocks[INDIRECT_BLOCK_IDX]);
            partition_write(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
            continue;
        }
        dir_inode->blocks[idx] = block_bitmap_idx;
        u32 bitmap_idx = onix_block_bitmap_alloc_sync(part);
        if (block_lba == -1)
        {
            bitmap_set(&part->block_bitmap, block_bitmap_idx, 0);
            dir_inode->blocks[idx] = 0;
            printk("alloc block bitmap failed\n");
            return false;
        }

        assert(idx == INDIRECT_BLOCK_IDX);
        blocks[idx] = block_bitmap_idx;
        flag = true;
        continue;
    }
    printk("directory is full!\n");
    return false;
}

void init_dir()
{
    root_dir.inode = NULL;
    open_root_dir(root_part);
}