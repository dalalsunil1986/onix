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
    if (buf == NULL)
    {
        return false;
    }

    Inode *inode = malloc(sizeof(Inode));
    if (inode == NULL)
    {
        free(buf);
        return;
    }

    memset(buf, 0, BLOCK_SIZE);
    memset(inode, 0, sizeof(Inode));

    bool success = false;
    u32 step = 0;

    u32 nr = onix_inode_bitmap_alloc(part);
    DEBUGP("inode alloc %u\n", nr);
    if (nr == -1)
    {
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

    onix_init_dir_entry(name, nr, FILETYPE_REGULAR, &entry);

    if (!onix_sync_dir_entry(part, parent, &entry))
    {
        printk("Onix sync directory fail!!!\n");
        step = 1;
        goto rollback;
    }
    onix_inode_sync(part, parent->inode);
    onix_inode_sync(part, inode);
    onix_bitmap_sync(part, nr, INODE_BITMAP);
    queue_push(&part->open_inodes, &inode->node);
    inode->open_cnts = 1;
    success = true;

rollback:
    switch (step)
    {
    case 1:
        onix_inode_bitmap_rollback(part, nr);
    default:
        break;
    }
    free(buf);
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
        return -1;
    }

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return -1;
    }

    u32 *new_blocks = malloc(ALL_BLOCKS_SIZE);
    if (new_blocks == NULL)
    {
        free(buf);
        return -1;
    }

#ifndef ONIX_KERNEL_DEBUG
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        free(buf);
        free(new_blocks);
        return -1;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif

    memset(buf, 0, BLOCK_SIZE);
    memset(new_blocks, 0, ALL_BLOCKS_SIZE);

    u32 bytes = 0;
    u32 step = 0;

    u32 block_start = file->offset / BLOCK_SIZE;                                       // 起始块
    u32 block_remain_bytes = file->offset % BLOCK_SIZE;                                // 起始块已占用空间
    u32 block_left_bytes = block_remain_bytes ? (BLOCK_SIZE - block_remain_bytes) : 0; // 起始块剩余空间

    Inode *inode = file->inode;

    u32 valid_blocks = onix_block_loads(part, inode, blocks);
    u32 used_blocks = block_start;
    u32 left_blocks = valid_blocks - used_blocks;

    u32 write_bytes = 0;

    u32 last_idx = block_start;

    if (block_left_bytes > 0 && blocks[last_idx])
    { // 写入不完整的最后一个扇区
        write_bytes = MIN(block_left_bytes, count);

        onix_block_read(part, blocks[last_idx], buf);
        memcpy(buf + block_remain_bytes, content, write_bytes);
        onix_block_write(part, blocks[last_idx], buf);

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

    if ((need_blocks + valid_blocks) > INDIRECT_BLOCK_IDX && inode->blocks[INDIRECT_BLOCK_IDX] == 0)
    {
        // 直接块不够写，需要间接块
        // 间接块不存在
        need_blocks++;
    }

    u32 block_idx = -1;
    u32 idx = 0;
    while (idx < need_blocks)
    {
        block_idx = onix_block_bitmap_alloc(part);
        if (block_idx == -1)
        {
            printk("Onix inode block allocate failed.\n");
            step = 1;
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
        if (count >= BLOCK_SIZE)
        {
            onix_block_write(part, blocks[idx], content);
            content += BLOCK_SIZE;
            bytes += BLOCK_SIZE;
            file->offset += BLOCK_SIZE;
            count -= BLOCK_SIZE;
        }
        else
        {
            memset(buf, 0, sizeof(buf));
            memcpy(buf, content, count);
            onix_block_write(part, blocks[idx], buf);
            bytes += count;
            file->offset += count;
            break;
        }
    }

    // 以下同步 inode->blocks
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
        else if (idx == INDIRECT_BLOCK_IDX && inode->blocks[idx] == 0)
        {
            inode->blocks[idx] = new_blocks[new_block_idx++];
        }

        onix_block_sync_indirect(part, inode->blocks[idx], blocks);
        break;
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

rollback:
    switch (step)
    {
    case 1:
        idx = 0;
        while (new_blocks[idx])
        {
            onix_block_bitmap_rollback(part, new_blocks[idx]);
            idx++;
        }
        bytes = -1;
    default:
        break;
    }
    free(buf);
    free(new_blocks);
#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif
    return bytes;
}

int32 onix_file_read(OnixFile *file, const void *content, int32 count)
{
    assert(count > 0);
    if (file->offset + count > MAX_FILE_SIZE)
    {
        printk("Exceed onix max file size...\n");
        return -1;
    }

    char *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        return -1;
    }

#ifndef ONIX_KERNEL_DEBUG
    u32 *blocks = malloc(ALL_BLOCKS_SIZE);
    if (blocks == NULL)
    {
        free(buf);
        return -1;
    }
#else
    u32 blocks[INODE_ALL_BLOCKS];
#endif

    Partition *part = file->part;
    u32 block_start = file->offset / BLOCK_SIZE; // 起始块

    assert(block_start < INODE_ALL_BLOCKS);

    u32 block_remain_bytes = file->offset % BLOCK_SIZE;                                // 起始块已占用空间
    u32 block_left_bytes = block_remain_bytes ? (BLOCK_SIZE - block_remain_bytes) : 0; // 起始块剩余空间

    Inode *inode = file->inode;

    u32 step = 0;
    int32 bytes = EOF;

    u32 valid_blocks = onix_block_loads(part, inode, blocks);

    if (!blocks[block_start])
    {
        step = 2;
        goto rollback;
    }

    u32 idx = block_start;

    if (block_remain_bytes)
    {
        onix_block_read(part, blocks[idx], buf);

        if (count <= block_left_bytes)
        {
            memcpy(content, buf + block_remain_bytes, count);
            file->offset += count;
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
        if (count >= BLOCK_SIZE)
        {
            onix_block_read(part, blocks[idx], content);
            content += BLOCK_SIZE;
            bytes += BLOCK_SIZE;
            file->offset += BLOCK_SIZE;
        }
        else
        {
            onix_block_read(part, blocks[idx], buf);
            memcpy(content, buf, count);
            bytes += count;
            file->offset += count;
            goto rollback;
        }
        idx++;
    }

rollback:
    free(buf);
#ifndef ONIX_KERNEL_DEBUG
    free(blocks);
#endif
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