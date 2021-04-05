#include <fs/onix/fsdir.h>
#include <fs/onix/inode.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsbitmap.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/assert.h>
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

Dir *onix_open_root_dir(Partition *part)
{
    if (root_dir.inode != NULL)
    {
        return &root_dir;
    }
    root_dir.inode = onix_inode_open(part, part->super_block->root_inode_nr);
    root_dir.dir_offset = 0;
    return &root_dir;
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
    dir->dir_offset = 0;
    return dir;
}

void onix_dir_close(Partition *part, Dir *dir)
{
    if (dir == &root_dir)
        return;
    onix_inode_close(part, dir->inode);
    free(dir);
}

bool onix_search_dir_entry(Partition *part, Dir *dir, char *name, DirEntry *entry)
{
    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32));
    u32 step = 0;
    bool success = false;
    if (blocks == NULL)
    {
        step = 1;
        goto rollback;
    }
    u8 *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        step = 2;
        goto rollback;
    }

    u32 idx = 0;
    u32 lba = 0;

    memcpy(blocks, dir->inode->blocks, INODE_BLOCK_CNT * sizeof(u32));
    u32 indirect_idx = dir->inode->blocks[INDIRECT_BLOCK_IDX];
    if (indirect_idx)
    {
        lba = get_block_lba(part, indirect_idx);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
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
                success = true;
                step = 3;
                goto rollback;
            }
            entry_idx++;
            dir_ptr++;
        }
        idx++;
        dir_ptr = buf;
        memset(buf, 0, BLOCK_SIZE);
    }

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
    assert(abuf[0] == '/' && pathlen > 1 && pathlen < MAX_PATH_LEN);

    char *subpath = abuf;
    Dir *parent = &root_dir;
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

        if (!onix_search_dir_entry(part, parent, name, entry))
        {
            step = 3;
            goto rollback;
        }

        if (entry->type == FILETYPE_REGULAR)
        {
            record->type = FILETYPE_REGULAR;
            ret = entry->inode_nr;
            step = 3;
            goto rollback;
        }

        if (entry->type == FILETYPE_DIRECTORY)
        {
            nr = parent->inode->nr;
            onix_dir_close(part, parent);
            parent = onix_dir_open(part, entry->inode_nr);
            record->parent = parent;
        }
        memset(name, 0, sizeof(name));
        if (subpath)
        {
            subpath = dirname(subpath, name);
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

void onix_create_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry)
{
    assert(strlen(filename) <= MAX_FILENAME_LENGTH);
    memset(entry->filename, 0, sizeof(entry->filename));
    memcpy(entry->filename, filename, strlen(filename));
    entry->inode_nr = nr;
    entry->type = type;
}

bool onix_sync_dir_entry(Partition *part, Dir *parent, DirEntry *entry)
{
    Inode *dir_inode = parent->inode;
    u32 dir_size = dir_inode->size;
    u32 dir_entry_size = root_part->super_block->dir_entry_size;

    assert(dir_size % dir_entry_size == 0);

    u32 dir_entry_cnt = (BLOCK_SIZE / dir_entry_size);
    int32 block_lba = -1;

    u32 idx = 0;
    u32 step = 0;
    bool success = false;
    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32)); //  todo rollback
    if (blocks == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(blocks, 0, INODE_ALL_BLOCKS * sizeof(u32));
    // u32 blocks[INODE_ALL_BLOCKS] = {0};

    memcpy(blocks, dir_inode->blocks, INODE_BLOCK_CNT * sizeof(u32));

    u32 lba = 0;
    u32 indirect_idx = dir_inode->blocks[INDIRECT_BLOCK_IDX];
    if (indirect_idx)
    {
        lba = get_block_lba(part, indirect_idx);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
    }

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        step = 2;
        goto rollback;
    }

    memset(buf, 0, BLOCK_SIZE);

    DirEntry *dir_entry = buf;

    int32 block_bitmap_idx = -1;

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
                    step = 3;
                    success = true;
                    goto rollback;
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
            step = 3;
            goto rollback;
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
            step = 3;
            goto rollback;
        }

        assert(idx == INDIRECT_BLOCK_IDX);
        dir_inode->blocks[idx] = block_bitmap_idx;
        onix_inode_sync(part, dir_inode);
        flag = true;
        continue;
    }
    printk("directory is full!\n");
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
    lba = get_block_lba(part, inode->blocks[INDIRECT_BLOCK_CNT]);
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

    memcpy(blocks, inode->blocks, INODE_BLOCK_CNT * sizeof(u32));

    u32 lba = 0;
    u32 indirect_idx = inode->blocks[INDIRECT_BLOCK_IDX];
    if (indirect_idx)
    {
        lba = get_block_lba(part, indirect_idx);
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

    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx] == 0)
        {
            idx++;
            continue;
        }

        partition_read(part, get_block_lba(part, blocks[idx]), buf, BLOCK_SECTOR_COUNT);
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
        partition_write(part, get_block_lba(part, blocks[idx]), buf, BLOCK_SECTOR_COUNT);

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

void init_dir()
{
    root_dir.inode = NULL;
    onix_open_root_dir(root_part);
}