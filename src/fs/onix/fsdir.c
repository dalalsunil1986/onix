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
        onix_block_sync_indirect(part, pinode->blocks[INDIRECT_BLOCK_IDX], blocks);
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

bool onix_search_dir_entry(Partition *part, Dir *parent, char *name, DirEntry *entry)
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
        return false;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif
    bool success = false;

    u32 idx = 0;
    u32 lba = 0;

    onix_block_loads(part, parent->inode, blocks);

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
                goto rollback;
            }
            entry_idx++;
        }
        idx++;
    }
rollback:
    free(buf);
#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif
    return success;
}

int32 onix_search_file(const char *pathname, SearchRecord *record)
{
    char *name = malloc(MAX_FILENAME_LENGTH);
    if (name == NULL)
    {
        return FILE_NULL;
    }
    char *path = malloc(MAX_PATH_LEN);
    if (path == NULL)
    {
        free(name);
        return FILE_NULL;
    }

    memset(path, 0, MAX_PATH_LEN);
    memset(name, 0, MAX_FILENAME_LENGTH);

    int32 ret = FILE_NULL;
    u32 step = 0;
    Partition *part = get_path_part(pathname);

    abspath(pathname, path);

    if (!strcmp(path, "/"))
    {
        record->parent = &root_dir;
        record->type = FILETYPE_DIRECTORY;
        memcpy(record->search_path, "/", 2);
        ret = 0;
        goto rollback;
    }

    u32 pathlen = strlen(path);
    assert(path[0] == '/' && pathlen >= 1 && pathlen < MAX_PATH_LEN);

    Dir *parent = &root_dir;
    u32 parent_nr = 0;

    DirEntry *entry = &record->entry;

    record->parent = parent;
    record->type = FILETYPE_UNKNOWN;

    char *subpath = dirname(path, name);

    while (name[0])
    {
        assert(strlen(record->search_path) < MAX_PATH_LEN);
        strcat(record->search_path, "/");
        strcat(record->search_path, name);

        if (!onix_search_dir_entry(part, parent, name, entry))
        {
            goto rollback;
        }
        if (entry->type == FILETYPE_REGULAR)
        {
            ret = entry->inode_nr;
            record->type = FILETYPE_REGULAR;
            goto rollback;
        }

        memset(name, 0, MAX_FILENAME_LENGTH);

        if (subpath)
        {
            subpath = dirname(subpath, name);
        }

        if (entry->type == FILETYPE_DIRECTORY)
        {
            parent_nr = parent->inode->nr;
            onix_dir_close(part, parent);
            record->type = FILETYPE_DIRECTORY;
            parent = onix_dir_open(part, entry->inode_nr);
            record->parent = parent;
        }
    }
    onix_dir_close(part, record->parent);
    record->parent = onix_dir_open(part, parent_nr);
    record->type = FILETYPE_DIRECTORY;
    ret = entry->inode_nr;

rollback:
    free(name);
    free(path);
    return ret;
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
    onix_delete_dir_entry(part, parent, entry->filename);
    onix_inode_delete(part, inode->nr);
    return 0;
}

bool onix_dir_empty(Partition *part, Dir *dir)
{
    Inode *inode = dir->inode;
    return (inode->size == part->super_block->dir_entry_size * 2);
}

static bool onix_delete_dir_block(Partition *part, Inode *inode, u32 *blocks, u32 block_idx)
{
    if (block_idx == 0) // 第一个块不能删
        return false;

    u32 idx = block_idx;
    u32 lba;

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
        {
            idx++;
            iblock_count++;
        }
    }

    assert(iblock_count >= 1);

    if (iblock_count > 1)
    {
        blocks[block_idx] = 0;
        onix_block_sync_indirect(part, inode->blocks[INDIRECT_BLOCK_IDX], blocks);
        return;
    }

    onix_block_bitmap_rollback_sync(part, blocks[block_idx]);
    inode->blocks[INDIRECT_BLOCK_IDX] = 0;
    return true;
}

bool onix_delete_dir_entry(Partition *part, Dir *parent, char *name)
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
        return false;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif
    bool success = false;
    Inode *inode = parent->inode;

    onix_block_loads(part, inode, blocks);

    DirEntry *entry = buf;
    DirEntry *found = NULL;
    u32 dir_entry_size = part->super_block->dir_entry_size;
    u32 dir_entry_cnt = BLOCK_SIZE / dir_entry_size;

    u32 idx = 0;
    u32 file_count = 0;
    u32 dir_count = 0;

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
            entry = (DirEntry *)buf + entry_idx;
            if (entry->type == FILETYPE_UNKNOWN)
            {
                entry_idx++;
                continue;
            }
            if (!strcmp(entry->filename, "."))
            {
                entry_idx++;
                continue;
            }
            if (!strcmp(entry->filename, ".."))
            {
                entry_idx++;
                continue;
            }
            if (!strcmp(entry->filename, name))
            {
                found = entry;
            }
            if (entry->type == FILETYPE_REGULAR)
            {
                file_count++;
                entry_idx++;
                continue;
            }
            else if (entry->type == FILETYPE_DIRECTORY)
            {
                dir_count++;
                entry_idx++;
                continue;
            }
            entry_idx++;
        }

        if (!found)
        {
            idx++;
            continue;
        }

        if (file_count + dir_count == 1)
        {
            onix_delete_dir_block(part, inode, blocks, idx);
        }

        // 删除目录项
        memset(found, 0, dir_entry_size);
        onix_block_write(part, blocks[idx], buf);

        // 同步父目录大小
        assert(inode->size % dir_entry_size == 0 && inode->size >= dir_entry_size);
        inode->size -= dir_entry_size;
        onix_inode_sync(part, inode);
        success = true;
        goto rollback;
    }
    printk("%s is not exists...\n", name);
rollback:
    free(buf);
#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif
    return success;
}
