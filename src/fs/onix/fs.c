#include <onix/fs.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/assert.h>
#include <onix/stdlib.h>
#include <onix/malloc.h>
#include <onix/string.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Partition *root_part;
extern Queue partition_queue;
char default_part[8] = "sdb1";

static void partition_mount(Node *node, char *partname)
{
    Partition *part = element_entry(Partition, node, node);

    if (strcmp(part->name, partname)) // 若不相等直接返回
    {
        DEBUGP("partition name %s not match %s\n", part->name, partname);
        return false;
    }

    DEBUGP("partition mount %s name %s\n", part->name, partname);

    root_part = part;
    Harddisk *disk = root_part->disk;
    SuperBlock *sb = malloc(sizeof(SuperBlock));

    // 读超级块
    root_part->super_block = malloc(sizeof(SuperBlock));
    if (root_part->super_block == NULL)
    {
        panic("allocate memory failed!");
    }
    memset(sb, 0, sizeof(SuperBlock));
    harddisk_read(disk, root_part->start_lba + 1, sb, 1);
    memcpy(root_part->super_block, sb, sizeof(SuperBlock));

    // 读块位图
    root_part->block_bitmap.bits = malloc(sb->block_bitmap_blocks * BLOCK_SIZE);
    if (root_part->block_bitmap.bits == NULL)
    {
        panic("allocate memory failed!");
    }

    root_part->block_bitmap.length = sb->block_bitmap_blocks * BLOCK_SIZE;
    harddisk_read(disk,
                  sb->block_bitmap_lba,
                  root_part->block_bitmap.bits,
                  sb->block_bitmap_blocks);

    // 读 inode 位图
    root_part->inode_bitmap.bits = malloc(sb->inode_bitmap_blocks * BLOCK_SIZE);
    if (root_part->inode_bitmap.bits == NULL)
    {
        panic("allocate memory failed!");
    }
    root_part->inode_bitmap.length = sb->inode_bitmap_blocks * BLOCK_SIZE;

    harddisk_read(disk,
                  sb->inode_bitmap_lba,
                  root_part->inode_bitmap.bits,
                  sb->inode_bitmap_blocks);

    queue_init(&root_part->open_inodes);
    free(sb);
    DEBUGP("mount %s done!\n", part->name);
    return true;
}

static void partition_format(Partition *part)
{
    u32 boot_sector_blocks = 1;
    u32 super_block_blocks = 1;
    u32 inode_bitmap_blocks = round_up(MAX_FILE_PER_PART, BLOCK_BITS);
    u32 inode_table_blocks = round_up((sizeof(Inode) * MAX_FILE_PER_PART), BLOCK_SIZE);

    u32 part_blocks = (part->sec_cnt * SECTOR_SIZE) / BLOCK_SIZE;
    u32 used_blocks = boot_sector_blocks + super_block_blocks +
                      inode_bitmap_blocks + inode_table_blocks;
    u32 free_blocks = part_blocks - used_blocks;

    u32 block_bitmap_blocks = round_up(free_blocks, BLOCK_BITS);
    u32 block_bitmap_bits = free_blocks - block_bitmap_blocks;
    block_bitmap_blocks = round_up(block_bitmap_bits, BLOCK_BITS);

    SuperBlock sb;
    sb.magic = FS_MAGIC;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILE_PER_PART;
    sb.start_lba = part->start_lba;

    sb.block_bitmap_lba = sb.start_lba + 2; // 第 0 块是引导块，第一块是超级块
    sb.block_bitmap_blocks = block_bitmap_blocks;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_blocks;
    sb.inode_bitmap_blocks = inode_bitmap_blocks;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_blocks;
    sb.inode_table_blocks = inode_table_blocks;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_blocks;

    sb.root_inode_nr = 0;
    sb.dir_entry_size = sizeof(DirEntry);

    DEBUGP("%s info:\n", part->name);
    DEBUGP("    magic:0x%08X\n", sb.magic);
    DEBUGP("    start_lba:0x%X\n", sb.start_lba);
    DEBUGP("    all_sectors:0x%X\n", sb.sec_cnt);
    DEBUGP("    inode_cnt:0x%X\n", sb.inode_cnt);
    DEBUGP("    block_bitmap_lba:0x%X\n", sb.block_bitmap_lba);
    DEBUGP("    block_bitmap_blocks:0x%X\n", sb.block_bitmap_blocks);
    DEBUGP("    inode_bitmap_lba:0x%X\n", sb.inode_bitmap_lba);
    DEBUGP("    inode_bitmap_blocks:0x%X\n", sb.inode_bitmap_blocks);
    DEBUGP("    inode_table_lba:0x%X\n", sb.inode_table_lba);
    DEBUGP("    inode_table_blocks:0x%X\n", sb.inode_table_blocks);
    DEBUGP("    data_start_lba:0x%X\n", sb.data_start_lba);
    DEBUGP("    super_block_lba:0x%X\n", part->start_lba + 1);

    Harddisk *disk = part->disk;
    harddisk_write(disk, part->start_lba + 1, &sb, 1);

    u32 buf_size = MAX(sb.block_bitmap_blocks, sb.inode_bitmap_blocks);
    buf_size = MAX(buf_size, sb.inode_table_blocks) * BLOCK_SIZE;

    DEBUGP("malloc size %d\n", buf_size);

    u8 *buf = malloc(buf_size);
    if (buf == NULL)
    {
        panic("Cannot allocate memory!!!");
    }
    memset(buf, 0, buf_size);

    DEBUGP("buffer %X\n", buf);

    /* 初始化块位图block_bitmap */
    buf[ROOT_DIR_IDX] |= 0x01; // 第0个块预留给根目录,位图中先占位
    u32 block_bitmap_last_byte = block_bitmap_bits / 8;
    u8 block_bitmap_last_bit = block_bitmap_bits % 8;
    u32 last_size = BLOCK_SIZE - (block_bitmap_last_byte % BLOCK_SIZE);
    // last_size是位图所在最后一个扇区中，不足一扇区的其余部分

    /* 1 先将位图最后一字节到其所在的扇区的结束全置为1,即超出实际块数的部分直接置为已占用*/
    memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    /* 2 再将上一步中覆盖的最后一字节内的有效位重新置0 */
    u8 idx = 0;
    while (idx <= block_bitmap_last_bit)
    {
        buf[block_bitmap_last_byte] &= ~(1 << idx++);
    }
    harddisk_write(disk, sb.block_bitmap_lba, buf, sb.block_bitmap_blocks);

    /*3 将inode位图初始化并写入sb.inode_bitmap_lba */

    memset(buf, 0, buf_size);
    buf[ROOT_DIR_IDX] |= 0x1; // 第0个inode分给了根目录
    harddisk_write(disk, sb.inode_bitmap_lba, buf, sb.inode_bitmap_blocks);

    /* 4 将inode数组初始化并写入sb.inode_table_lba */

    /* 准备写inode_table中的第0项,即根目录所在的inode */
    memset(buf, 0, buf_size); // 先清空缓冲区buf
    Inode *i = buf;
    i->size = sb.dir_entry_size * 2;  // .和..
    i->nr = 0;                        // 根目录占inode数组中第0个inode
    i->blocks[0] = sb.data_start_lba; // 由于上面的memset,i_sectors数组的其它元素都初始化为0
    harddisk_write(disk, sb.inode_table_lba, buf, sb.inode_table_blocks);

    /* 5 将根目录初始化并写入sb.data_start_lba */
    /* 写入根目录的两个目录项.和.. */
    memset(buf, 0, buf_size);
    DirEntry *entry = buf;

    memcpy(entry->filename, ".", 1);
    entry->inode_nr = ROOT_DIR_IDX;
    entry->type = FILETYPE_DIRECTORY;
    entry++;

    memcpy(entry->filename, "..", 2);
    entry->inode_nr = ROOT_DIR_IDX; // 根目录的父目录依然是根目录自己
    entry->type = FILETYPE_DIRECTORY;
    harddisk_write(disk, sb.data_start_lba, buf, 1);

    DEBUGP("%s format done\n", part->name);
    free(buf);
}

static void search_part_fs(Harddisk *disk, Partition *part)
{
    if (part->sec_cnt <= 0)
        return;

    SuperBlock *sb = malloc(SECTOR_SIZE);
    if (sb == NULL)
    {
        panic("alloc memory failed!!!");
    }

    DEBUGP("search %s part %s\n", disk->name, part->name);
    memset(sb, 0, SECTOR_SIZE);
    harddisk_read(disk, part->start_lba + 1, sb, 1);
    if (sb->magic == FS_MAGIC)
    {
        DEBUGP("%s has file system\n", part->name);
        // partition_format(part);
    }
    else
    {
        DEBUGP("formating %s's partition %s....\n", disk->name, part->name);
        partition_format(part);
    }
    free(sb);
}

static void search_disk_fs(Harddisk *disk)
{
    Partition *part = disk->primary_parts;
    u8 idx = 0;
    while (idx < MAX_PART)
    {
        if (idx == MAX_PRIMARY_PART)
        {
            part = disk->logical_parts;
        }
        search_part_fs(disk, part);
        part++;
        idx++;
    }
}

static void search_channel_fs(IDEChannel *channel)
{
    u8 idx = 1;
    while (idx < 2)
    {
        Harddisk *disk = channel->devices + idx;
        DEBUGP("search disk %s \n", disk->name);
        search_disk_fs(disk);
        idx++;
    }
}

void init_fs()
{
    extern u8 channel_count;

    u8 idx = 0;
    DEBUGP("searching filesystem in %d channel.....\n", channel_count);
    while (idx < channel_count)
    {
        IDEChannel *channel = channels + idx;
        search_channel_fs(channel);
        idx++;
    }

    queue_traversal(&partition_queue, partition_mount, default_part);
}