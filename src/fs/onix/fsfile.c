#include <fs/onix/fsfile.h>
#include <fs/onix/fsbitmap.h>
#include <fs/onix/inode.h>
#include <fs/onix/fsdir.h>
#include <fs/onix/fsblock.h>
#include <fs/file.h>
#include <onix/malloc.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/kernel/interrupt.h>
#include <onix/assert.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

bool onix_file_create(Partition *part, Dir *parent, OnixFile *file, char *name, FileFlag flags)
{
    void *buf = malloc(BLOCK_SIZE);
    bool success = false;
    u32 step = 0;
    if (buf == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(buf, 0, BLOCK_SIZE);

    u32 nr = onix_inode_bitmap_alloc(part);
    DEBUGP("inode alloc %u\n", nr);
    if (nr == -1)
    {
        step = 2;
        goto rollback;
    }

    Inode *inode = malloc(sizeof(Inode));
    memset(inode, 0, sizeof(Inode));

    if (inode == NULL)
    {
        step = 3;
        goto rollback;
    }

    onix_inode_init(nr, inode);

    file->inode = inode;
    file->offset = 0;
    file->inode->write_deny = false;
    file->flags = flags;
    file->part = part;

    DirEntry entry;
    memset(&entry, 0, sizeof(DirEntry));

    onix_create_dir_entry(name, nr, FILETYPE_REGULAR, &entry);

    if (!onix_sync_dir_entry(part, parent, &entry))
    {
        printk("Onix sync directory fail!!!\n");
        step = 3;
        goto rollback;
    }
    onix_inode_sync(part, parent->inode);
    onix_inode_sync(part, inode);
    onix_bitmap_sync(part, nr, INODE_BITMAP);
    queue_push(&part->open_inodes, &inode->node);
    inode->open_cnts = 1;
    success = true;
    step = 2;

rollback:
    switch (step)
    {
    case 3:
        onix_inode_bitmap_rollback(part, nr);
    case 2:
        free(buf);
    case 1:
        break;
    default:
        break;
    }
    return success;
}

bool onix_file_open(Partition *part, OnixFile *file, u32 nr, FileFlag flags)
{
    file->inode = onix_inode_open(part, nr);
    file->offset = 0;
    file->flags = flags;
    file->part = part;

    if (!(flags & O_W || flags & O_RW)) // 如果不写
        return true;
    bool success = false;
    bool *write_deny = &file->inode->write_deny;
    bool old = disable_int();
    if (!*write_deny)
    {
        *write_deny = true;
        success = true;
    }
    else
    {
        printk("file can't write now, try again later\n");
    }
    set_interrupt_status(old);
    return success;
}

bool onix_file_close(OnixFile *file)
{
    if (file == NULL)
        return false;
    file->inode->write_deny = false;
    onix_inode_close(file->part, file->inode);
    file->inode = NULL;
    return true;
}

int32 onix_file_write(OnixFile *file, const void *content, int32 count)
{
    Partition *part = file->part;

    assert(count > 0);
    if (file->offset + count > MAX_FILE_SIZE)
    {
        printk("Exceed onix max file size...\n");
        return 0;
    }

    u32 bytes = 0;
    u32 step = 0;

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(buf, 0, BLOCK_SIZE);

    u32 block_start = file->offset / BLOCK_SIZE;                                       // 起始块
    u32 block_remain_bytes = file->offset % BLOCK_SIZE;                                // 起始块已占用空间
    u32 block_left_bytes = block_remain_bytes ? (BLOCK_SIZE - block_remain_bytes) : 0; // 起始块剩余空间

    u32 idx = 0;
    Inode *inode = file->inode;

    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32));
    if (blocks == NULL)
    {
        step = 2;
        goto rollback;
    }

    memset(blocks, 0, INODE_ALL_BLOCKS * sizeof(u32));

    while (idx < DIRECT_BLOCK_CNT && inode->blocks[idx] > 0)
    {
        blocks[idx] = inode->blocks[idx];
        idx++;
    }

    u32 used_direct_blocks = idx;

    u32 lba = 0;
    if (inode->blocks[INDIRECT_BLOCK_IDX])
    {
        // 一级间接表存在，读入；
        lba = onix_block_lba(part, inode->blocks[INDIRECT_BLOCK_IDX]);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
        while (blocks[idx])
        {
            idx++;
        }
    }

    u32 valid_blocks = idx;
    u32 used_blocks = block_start;
    u32 left_blocks = valid_blocks - used_blocks;

    u32 write_bytes = 0;

    u32 last_idx = block_start;

    if (block_left_bytes > 0 && blocks[last_idx])
    { // 写入不完整的最后一个扇区
        write_bytes = MIN(block_left_bytes, count);

        lba = onix_block_lba(part, blocks[last_idx]);

        partition_read(part, lba, buf, BLOCK_SECTOR_COUNT);

        memcpy(buf + block_remain_bytes, content, write_bytes);

        partition_write(part, lba, buf, BLOCK_SECTOR_COUNT);

        bytes = write_bytes;
        content += bytes;
        count -= bytes;
        if (count <= 0)
        {
            // 写完了
            file->offset += bytes;
            goto sync_inode;
        }
        left_blocks--;
        last_idx++;
    }

    // 总共需要这么多的块来写入文件
    u32 total_blocks = round_up(count - block_left_bytes, BLOCK_SIZE);
    int32 need_blocks = total_blocks - left_blocks;

    u32 free_direct_blocks = DIRECT_BLOCK_CNT - used_direct_blocks;

    if (need_blocks > 0 && (free_direct_blocks < need_blocks) && !inode->blocks[INDIRECT_BLOCK_IDX])
    {
        // 直接块不够写，需要间接块
        // 间接块不存在
        need_blocks++;
    }

    u32 new_blocks[INODE_ALL_BLOCKS] = {0};
    u32 block_idx = -1;
    idx = 0;
    while (idx < need_blocks)
    {
        block_idx = onix_inode_bitmap_alloc(part);
        if (block_idx == -1)
        {
            printk("Onix inode block allocate failed.\n");
            step = 4;
            goto rollback;
        }
        new_blocks[idx] = block_idx;
        idx++;
    }

    u32 new_block_idx = 0;
    idx = last_idx;

    while (idx < INODE_ALL_BLOCKS)
    {
        if (blocks[idx] == 0)
        {
            blocks[idx] = new_blocks[new_block_idx++];
        }
        lba = onix_block_lba(part, blocks[idx]);
        if (count >= BLOCK_SIZE)
        {
            partition_write(part, lba, content, BLOCK_SECTOR_COUNT);
            content += BLOCK_SIZE;
            bytes += BLOCK_SIZE;
            file->offset += BLOCK_SIZE;
            count -= BLOCK_SIZE;
        }
        else
        {
            memset(buf, 0, sizeof(buf));
            memcpy(buf, content, count);
            partition_write(part, lba, buf, BLOCK_SECTOR_COUNT);
            bytes += count;
            file->offset += count;
            break;
        }
    }

    idx = 0;
    bool flag = false;
    while (idx < INODE_ALL_BLOCKS)
    {
        if (!blocks[idx])
            break;
        if (idx < DIRECT_BLOCK_CNT)
        {
            inode->blocks[idx] = blocks[idx];
            idx++;
            continue;
        }
        else if (flag && idx >= INDIRECT_BLOCK_IDX)
        {
            lba = onix_block_lba(part, inode->blocks[INDIRECT_BLOCK_IDX]);
            partition_write(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
            break;
        }

        assert(idx == INDIRECT_BLOCK_IDX);
        if (blocks[idx] == 0)
        {
            blocks[idx] = new_blocks[new_block_idx++];
            flag = true;
        }
    }
sync_inode:
    if (file->offset > file->inode->size)
    {
        file->inode->size = file->offset;
    }
    onix_inode_sync(part, inode);
    idx = 0;
    while (new_blocks[idx])
    {
        onix_bitmap_sync(part, new_blocks[idx], BLOCK_BITMAP);
        idx++;
    }
    step = 3;

rollback:
    switch (step)
    {
    case 4:
        idx = 0;
        while (new_blocks[idx])
        {
            onix_block_bitmap_rollback(part, new_blocks[idx]);
            idx++;
        }
    case 3:
        free(blocks);
    case 2:
        free(buf);
    default:
        break;
    }
}

int32 onix_file_read(OnixFile *file, const void *content, int32 count)
{
    Partition *part = file->part;

    assert(count > 0);

    u32 block_start = file->offset / BLOCK_SIZE; // 起始块

    assert(block_start < INODE_ALL_BLOCKS);

    u32 block_remain_bytes = file->offset % BLOCK_SIZE;                                // 起始块已占用空间
    u32 block_left_bytes = block_remain_bytes ? (BLOCK_SIZE - block_remain_bytes) : 0; // 起始块剩余空间

    Inode *inode = file->inode;

    u32 step = 0;
    int32 bytes = EOF;

    u32 *blocks = malloc(INODE_ALL_BLOCKS * sizeof(u32));
    if (blocks == NULL)
    {
        step = 1;
        goto rollback;
    }
    memcpy(blocks, inode->blocks, DIRECT_BLOCK_CNT * sizeof(u32));

    u32 lba = 0;
    if (inode->blocks[INDIRECT_BLOCK_IDX])
    {
        // 一级间接表存在，读入；
        lba = onix_block_lba(part, inode->blocks[INDIRECT_BLOCK_IDX]);
        partition_read(part, lba, blocks + INDIRECT_BLOCK_IDX, BLOCK_SECTOR_COUNT);
    }

    if (!blocks[block_start])
    {
        step = 2;
        goto rollback;
    }

    u32 idx = block_start;

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        step = 2;
        goto rollback;
    }
    memset(buf, 0, BLOCK_SIZE);

    if (block_remain_bytes)
    {
        lba = onix_block_lba(part, blocks[idx]);
        partition_read(part, lba, buf, BLOCK_SECTOR_COUNT);
        if (count <= block_left_bytes)
        {
            memcpy(content, buf + block_remain_bytes, count);
            file->offset += count;
            step = 3;
            bytes = count;
            goto rollback;
        }
        memcpy(content, buf + block_remain_bytes, block_left_bytes);
        count -= block_left_bytes;
        content += block_left_bytes;
        bytes += block_left_bytes;
        file->offset += block_left_bytes;
        idx++;
    }
    while (blocks[idx])
    {
        lba = onix_block_lba(part, blocks[idx]);
        if (count >= BLOCK_SIZE)
        {
            partition_read(part, lba, content, BLOCK_SECTOR_COUNT);
            content += BLOCK_SIZE;
            bytes += BLOCK_SIZE;
            file->offset += BLOCK_SIZE;
        }
        else
        {
            partition_read(part, lba, buf, BLOCK_SECTOR_COUNT);
            memcpy(content, buf, count);
            bytes += count;
            file->offset += count;
            step = 3;
            goto rollback;
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
        free(blocks);
    case 1:
        break;
    default:
        break;
    }

    return bytes;
}

int32 onix_file_lseek(OnixFile *file, int32 offset, Whence whence)
{
    u32 filesize = file->inode->size;
    u32 pos = 0;
    switch (whence)
    {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = file->offset + offset;
        break;
    case SEEK_END:
        pos = filesize + offset;
    default:
        break;
    }
    if (pos < 0 || pos > (filesize - 1))
    {
        return EOF;
    }
    file->offset = pos;
    return file->offset;
}