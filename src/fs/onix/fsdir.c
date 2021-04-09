#include <fs/onix/fsdir.h>
#include <fs/onix/inode.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/assert.h>
#include <onix/string.h>
#include <onix/kernel/debug.h>
#include <fs/path.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Dir root_dir;
extern Partition *root_part;

void init_dir()
{
    root_dir.inode = NULL;
    onix_open_root_dir(root_part);
}

Dir *onix_open_root_dir(Partition *part)
{
    if (root_dir.inode != NULL)
    {
        return &root_dir;
    }
    root_dir.inode = onix_inode_open(part, part->super_block->root_inode_nr);
    root_dir.offset = 0;
    root_dir.part = part;
    return &root_dir;
}

void onix_init_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry)
{
    assert(strlen(filename) <= MAX_FILENAME_LENGTH);
    memset(entry->filename, 0, sizeof(entry->filename));
    memcpy(entry->filename, filename, strlen(filename));
    entry->inode_nr = nr;
    entry->type = type;
}

static bool onix_commit_dir_entry(Partition *part, Inode *pinode, u32 idx, DirEntry *entry, void *buf)
{
    u32 dir_size = pinode->size;
    u32 dir_entry_size = part->super_block->dir_entry_size;
    u32 dir_entry_cnt = (BLOCK_SIZE / dir_entry_size);

    assert(dir_size % dir_entry_size == 0);

    onix_block_read(part, idx, buf);

    u32 pidx = 0;
    while (pidx < dir_entry_cnt)
    {
        DirEntry *pentry = (DirEntry *)buf + pidx;
        if (pentry->type == FILETYPE_UNKNOWN)
        {
            memcpy(pentry, entry, dir_entry_size);
            onix_block_write(part, idx, buf);
            pinode->size += dir_entry_size;
            return true;
        }
        pidx++;
    }
    return false;
}

bool onix_sync_dir_entry(Partition *part, Dir *parent, DirEntry *entry)
{
    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return false;
    }

#ifndef ONIX_KERNEL_DEBUG
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        free(buf);
        return false;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif

    u32 idx = 0;
    u32 step = 0;
    bool success = false;

    Inode *pinode = parent->inode;

    onix_block_loads(part, pinode, blocks);

    int32 block_bitmap_idx = -1;
    idx = 0;
    bool flag = false;

    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx] && onix_commit_dir_entry(part, pinode, blocks[idx], entry, buf))
        {
            success = true;
            onix_inode_sync(part, pinode);
            goto rollback;
        }

        if (blocks[idx]) // 有 block 但是已经满了的情况
        {
            idx++;
            continue;
        }

        block_bitmap_idx = onix_block_bitmap_alloc_sync(part);
        if (block_bitmap_idx == -1)
        {
            printk("allocate block bitmap failed ...\n");
            goto rollback;
        }

        if (idx < DIRECT_BLOCK_CNT) // 直接块
        {
            pinode->blocks[idx] = blocks[idx] = block_bitmap_idx;
            continue;
        }
        else if (idx == INDIRECT_BLOCK_IDX && !flag) // 一级间接块
        {
            pinode->blocks[idx] = block_bitmap_idx;
            onix_inode_sync(part, pinode);
            flag = true;
            continue;
        }

        blocks[idx] = block_bitmap_idx;
        char *blockbuf = (u32 *)blocks + INDIRECT_BLOCK_IDX;
        onix_block_write(part, pinode->blocks[INDIRECT_BLOCK_IDX], blockbuf);
        continue;
    }
    printk("directory is full!\n");

rollback:
    free(buf);
#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif
    return success;
}

Dir *onix_dir_open(Partition *part, u32 nr)
{
    Dir *dir = malloc(sizeof(Dir));
    if (dir == NULL)
    {
        printk("dir open allocate memroy fail!!!\n");
        return NULL;
    }
    dir->inode = onix_inode_open(part, nr);
    dir->offset = 0;
    dir->part = part;
    return dir;
}

void onix_dir_close(Partition *part, Dir *dir)
{
    if (dir == &root_dir)
        return;
    onix_inode_close(part, dir->inode);
    free(dir);
}

DirEntry *onix_dir_read(Partition *part, Dir *dir)
{
#ifndef ONIX_KERNEL_DEBUG
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        return NULL;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif

    DirEntry *entry = dir->buffer;
    Inode *inode = dir->inode;
    char *buf = dir->buffer;

    onix_block_loads(part, inode, blocks);

    u32 step = 0;
    bool success = false;
    u32 lba = 0;

    u32 entry_size = part->super_block->dir_entry_size;
    u32 entry_cnt = BLOCK_SIZE / entry_size;
    u32 entry_offset = 0;
    u32 idx = 0;

    while (idx < INODE_ALL_BLOCKS)
    {
        if (dir->offset >= inode->size)
        {
            entry = NULL;
            goto rollback;
        }
        if (blocks[idx] == 0)
        {
            idx++;
            continue;
        }

        onix_block_read(part, blocks[idx], dir->buffer);

        u32 entry_idx = 0;
        while (entry_idx < entry_cnt)
        {
            entry = (DirEntry *)dir->buffer + entry_idx;
            if (entry->type == FILETYPE_UNKNOWN)
            {
                entry_idx++;
                continue;
            }
            if (entry_offset < dir->offset)
            {
                entry_offset += entry_size;
                entry_idx++;
                continue;
            }
            assert(entry_offset == dir->offset);
            dir->offset += entry_size;
            goto rollback;
        }
        idx++;
    }
    entry = NULL;
rollback:

#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif

    return entry;
}

bool onix_search_dir_entry(Partition *part, Dir *dir, char *name, DirEntry *entry)
{
    u8 *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return false;
    }
#ifndef ONIX_KERNEL_DEBUG
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        free(buf);
        return NULL;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif

    u32 step = 0;
    bool success = false;

    u32 idx = 0;
    u32 lba = 0;

    onix_block_loads(part, dir->inode, blocks);

    DirEntry *pentry = buf;
    u32 dir_entry_size = part->super_block->dir_entry_size;
    u32 dir_entry_cnt = BLOCK_SIZE / dir_entry_size;

    idx = 0;
    while (idx < INODE_ALL_BLOCKS)
    {
        if (!blocks[idx])
        {
            idx++;
            continue;
        }
        onix_block_read(part, blocks[idx], buf);

        u32 entry_idx = 0;
        while (entry_idx < dir_entry_cnt)
        {
            pentry = (DirEntry *)buf + entry_idx;
            if (!strcmp(pentry->filename, name))
            {
                memcpy(entry, pentry, dir_entry_size);
                success = true;
                step = 3;
                goto rollback;
            }
            entry_idx++;
        }
        idx++;
    }
    step = 3;
rollback:
    switch (step)
    {
    case 3:
        free(buf);
    case 2:
#ifndef ONIX_KERNEL_DEBUG
        free(blocks);
#endif
    default:
        break;
    }
    return success;
}

bool onix_dir_remove(Partition *part, Dir *parent, Dir *dir, DirEntry *entry)
{
    Inode *inode = dir->inode;
    u32 idx = 1;

    while (idx < INODE_BLOCK_CNT)
    {
        assert(inode->blocks[idx] == 0);
        idx++;
    }

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return -1;
    }

    onix_delete_dir_entry(part, parent, entry);
    onix_inode_delete(part, inode->nr);
    free(buf);
    return 0;
}

bool onix_dir_empty(Partition *part, Dir *dir)
{
    Inode *inode = dir->inode;
    return (inode->size == part->super_block->dir_entry_size * 2);
}

int32 onix_search_file(const char *pathname, SearchRecord *record)
{
    // PBMB;
    char *abuf = malloc(MAX_PATH_LEN);
    int32 ret = FILE_NULL;
    u32 step = 0;
    if (abuf == NULL)
    {
        step = 1;
        goto rollback;
    }

    abspath(pathname, abuf);
    if (!strcmp(abuf, "/"))
    {
        record->parent = &root_dir;
        record->type = FILETYPE_DIRECTORY;
        record->search_path[0] = 0;
    }
    Partition *part = get_path_part(pathname);

    u32 pathlen = strlen(abuf);
    assert(abuf[0] == '/' && pathlen >= 1 && pathlen < MAX_PATH_LEN);

    char *subpath = abuf;

    Dir *parent = &root_dir;
    Dir *current = &root_dir;
    Dir *next = &root_dir;

    DirEntry *entry = &record->entry;

    char *name = malloc(MAX_FILENAME_LENGTH);
    if (name == NULL)
    {
        step = 2;
        goto rollback;
    }

    memset(name, 0, MAX_FILENAME_LENGTH);

    record->parent = parent;
    record->type = FILETYPE_UNKNOWN;

    u32 nr = 0;

    subpath = dirname(subpath, name);
    while (name[0])
    {
        assert(strlen(record->search_path) < MAX_PATH_LEN);
        strcat(record->search_path, "/");
        strcat(record->search_path, name);

        if (!onix_search_dir_entry(part, next, name, entry))
        {
            onix_dir_close(part, parent);
            onix_dir_close(part, current);
            record->parent = next;
            step = 3;
            ret = FILE_NULL;
            goto rollback;
        }

        if (entry->type == FILETYPE_REGULAR)
        {
            onix_dir_close(part, parent);
            onix_dir_close(part, current);
            record->parent = next;
            ret = entry->inode_nr;
            record->type = FILETYPE_REGULAR;
            step = 3;
            goto rollback;
        }

        if (entry->type == FILETYPE_DIRECTORY)
        {
            onix_dir_close(part, parent);
            parent = current;
            current = next;
            record->type = FILETYPE_DIRECTORY;
            next = onix_dir_open(part, entry->inode_nr);
            ret = entry->inode_nr;
        }
        memset(name, 0, sizeof(name));
        if (subpath)
        {
            subpath = dirname(subpath, name);
        }
        if (!name[0])
        {
            onix_dir_close(part, parent);
            record->parent = current;
            onix_dir_close(part, next);
        }
    }
    step = 3;
rollback:
    switch (step)
    {
    case 3:
        free(name);
    case 2:
        free(abuf);
    case 1:
        break;
    default:
        break;
    }
    return ret;
}

static bool onix_delete_dir_block(Partition *part, Inode *inode, u32 *blocks, u32 block_idx)
{
    u32 idx = block_idx;
    u32 lba;
    if (inode->nr == part->super_block->root_inode_nr && idx == 0)
        // 根目录的第一个块不能删
        return;

    onix_block_bitmap_rollback_sync(part, idx);

    if (idx < DIRECT_BLOCK_CNT)
    {
        inode->blocks[idx] = 0;
        return true;
    }
    idx = INDIRECT_BLOCK_IDX;
    u32 iblock_count = 0;
    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx])
            iblock_count++;
    }
    if (iblock_count == 1)
    {
        inode->blocks[INDIRECT_BLOCK_IDX] = 0;
        return;
    }
    blocks[block_idx] = 0;
    lba = onix_block_lba(part, inode->blocks[INDIRECT_BLOCK_CNT]);
    partition_write(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SIZE);
    return true;
}

bool onix_delete_dir_entry(Partition *part, Dir *parent, DirEntry *entry)
{
    assert(strcmp(entry->filename, "."));  // 不是当前目录
    assert(strcmp(entry->filename, "..")); // 不是父目录

    Inode *inode = parent->inode;
    u32 step = 0;
    bool success = false;

    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32));
    if (blocks == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(blocks, 0, INODE_ALL_BLOCKS * sizeof(u32));

    memcpy(blocks, inode->blocks, DIRECT_BLOCK_CNT * sizeof(u32));

    u32 lba = 0;
    u32 indirect_idx = inode->blocks[INDIRECT_BLOCK_IDX];
    if (indirect_idx)
    {
        lba = onix_block_lba(part, indirect_idx);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
    }

    char *buf = malloc(BLOCK_SIZE);
    if (blocks == NULL)
    {
        step = 2;
        goto rollback;
    }

    memset(buf, 0, BLOCK_SIZE);

    DirEntry *dir_ptr = buf;
    DirEntry *found_entry = NULL;

    u32 dir_entry_size = part->super_block->dir_entry_size;
    u32 dir_entry_cnt = BLOCK_SIZE / dir_entry_size;

    u32 idx = 0;

    u32 block_cnt = INODE_ALL_BLOCKS;

    while (idx < block_cnt)
    {
        if (blocks[idx] == 0)
        {
            idx++;
            continue;
        }

        partition_read(part, onix_block_lba(part, blocks[idx]), buf, BLOCK_SECTOR_COUNT);
        u32 entry_idx = 0;
        u32 file_count = 0;
        u32 dir_count = 0;

        while (entry_idx < dir_entry_cnt)
        {
            dir_ptr = buf + dir_entry_size * entry_idx;
            if (dir_ptr->type == FILETYPE_UNKNOWN)
            {
                entry_idx++;
                continue;
            }
            if (!strcmp(dir_ptr->filename, "."))
            {
                entry_idx++;
                continue;
            }
            if (!strcmp(dir_ptr->filename, ".."))
            {
                entry_idx++;
                continue;
            }

            if (!strcmp(dir_ptr->filename, entry->filename))
            {
                found_entry = dir_ptr;
            }
            if (dir_ptr->type == FILETYPE_REGULAR)
            {
                file_count++;
                entry_idx++;
                continue;
            }
            else if (dir_ptr->type == FILETYPE_DIRECTORY)
            {
                dir_count++;
                entry_idx++;
                continue;
            }
        }

        if (!found_entry)
        {
            idx++;
            continue;
        }

        if (file_count + dir_count == 1)
        {
            onix_delete_dir_block(part, inode, blocks, idx);
        }
        // 删除目录项
        memset(found_entry, 0, dir_entry_size);
        partition_write(part, onix_block_lba(part, blocks[idx]), buf, BLOCK_SECTOR_COUNT);

        // 同步父目录大小
        assert(inode->size >= dir_entry_size);
        inode->size -= dir_entry_size;
        onix_inode_sync(part, inode);
        step = 3;
        success = true;
        goto rollback;
    }
    step = 3;

rollback:
    switch (step)
    {
    case 3:
        free(buf);
    case 2:
        free(blocks);
    case 1:
        break;
    default:
        break;
    }
    return success;
}
