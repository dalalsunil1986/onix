#include <fs/onix/fs.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/harddisk.h>
#include <onix/assert.h>
#include <onix/stdlib.h>
#include <onix/malloc.h>
#include <onix/string.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

Partition *root_part;
extern Queue partition_queue;
char default_part[8] = "sdb1";

void print_format_info(Partition *part, SuperBlock *sb)
{
    u32 part_blocks = (part->sec_cnt * SECTOR_SIZE) / BLOCK_SIZE;
    u32 used_blocks = 2 + sb->inode_bitmap_blocks + sb->inode_table_blocks;
    u32 free_blocks = part_blocks - used_blocks;
    DEBUGK("%s info:\n", part->name);
    DEBUGK("    magic:0x%08X\n", sb->magic);
    DEBUGK("    start_lba:0x%X\n", sb->start_lba);
    DEBUGK("    all_sectors:0x%x\n", sb->sec_cnt);
    DEBUGK("    part_blocks:0x%x\n", part_blocks);
    DEBUGK("    used_blocks:0x%x\n", used_blocks);
    DEBUGK("    free_blocks:0x%x\n", free_blocks);
    DEBUGK("    inode_cnt:0x%x\n", sb->inode_cnt);
    DEBUGK("    block_bitmap_lba:0x%X\n", sb->block_bitmap_lba);
    DEBUGK("    block_bitmap_blocks:0x%X\n", sb->block_bitmap_blocks);
    DEBUGK("    inode_bitmap_lba:0x%X\n", sb->inode_bitmap_lba);
    DEBUGK("    inode_bitmap_blocks:0x%X\n", sb->inode_bitmap_blocks);
    DEBUGK("    inode_table_lba:0x%X\n", sb->inode_table_lba);
    DEBUGK("    inode_table_blocks:0x%X\n", sb->inode_table_blocks);
    DEBUGK("    data_start_lba:0x%X\n", sb->data_start_lba);
    DEBUGK("    super_block_lba:0x%X\n", part->start_lba + BLOCK_SECTOR_COUNT);
}

static bool partition_mount(Node *node, char *partname)
{
    Partition *part = element_entry(Partition, node, node);

    if (strcmp(part->name, partname)) // 若不相等直接返回
    {
        DEBUGP("partition name %s not match %s\n", part->name, partname);
        return false;
    }

    DEBUGP("partition mount %s name %s\n", part->name, partname);

    root_part = part;
    Harddisk *disk = part->disk;

    SuperBlock *sb = malloc(sizeof(SuperBlockHolder));

    // 读超级块
    part->super_block = malloc(sizeof(SuperBlockHolder));
    if (part->super_block == NULL)
    {
        panic("allocate memory failed!");
    }
    memset(sb, 0, sizeof(SuperBlock));
    partition_read(part, BLOCK_SECTOR_COUNT, sb, BLOCK_SECTOR_COUNT);
    memcpy(part->super_block, sb, sizeof(SuperBlockHolder));

    // 读块位图
    part->block_bitmap.bits = malloc(sb->block_bitmap_blocks * BLOCK_SIZE);
    if (part->block_bitmap.bits == NULL)
    {
        panic("allocate memory failed!");
    }

    part->block_bitmap.length = sb->block_bitmap_blocks * BLOCK_SIZE;
    partition_read(part,
                   sb->block_bitmap_lba,
                   part->block_bitmap.bits,
                   sb->block_bitmap_blocks * BLOCK_SECTOR_COUNT);

    // 读 inode 位图
    part->inode_bitmap.bits = malloc(sb->inode_bitmap_blocks * BLOCK_SIZE);
    if (part->inode_bitmap.bits == NULL)
    {
        panic("allocate memory failed!");
    }
    part->inode_bitmap.length = sb->inode_bitmap_blocks * BLOCK_SIZE;

    partition_read(part,
                   sb->inode_bitmap_lba,
                   part->inode_bitmap.bits,
                   sb->inode_bitmap_blocks * BLOCK_SECTOR_COUNT);

    queue_init(&part->open_inodes);
    free(sb);
    DEBUGK("mount %s done!\n", part->name);
    return true;
}

static void partition_format(Partition *part)
{
    u32 boot_sector_blocks = 1; // 后面会剩余若干扇区，不过无所谓
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

    SuperBlockHolder holder;
    SuperBlock *sb = &holder.m.super_block;

    sb->magic = FS_MAGIC;
    sb->sec_cnt = part->sec_cnt;
    sb->inode_cnt = MAX_FILE_PER_PART;
    sb->start_lba = part->start_lba;

    sb->block_bitmap_lba = ((boot_sector_blocks + super_block_blocks) * BLOCK_SECTOR_COUNT);
    sb->block_bitmap_blocks = block_bitmap_blocks;

    sb->inode_bitmap_lba = sb->block_bitmap_lba + sb->block_bitmap_blocks * BLOCK_SECTOR_COUNT;
    sb->inode_bitmap_blocks = inode_bitmap_blocks;

    sb->inode_table_lba = sb->inode_bitmap_lba + sb->inode_bitmap_blocks * BLOCK_SECTOR_COUNT;
    sb->inode_table_blocks = inode_table_blocks;

    sb->data_start_lba = sb->inode_table_lba + sb->inode_table_blocks * BLOCK_SECTOR_COUNT;

    sb->root_inode_nr = 0;
    sb->dir_entry_size = sizeof(DirEntry);

    print_format_info(part, sb);

    Harddisk *disk = part->disk;
    partition_write(part, BLOCK_SECTOR_COUNT, sb, BLOCK_SECTOR_COUNT);

    u32 buf_size = MAX(sb->block_bitmap_blocks, sb->inode_bitmap_blocks);
    buf_size = MAX(buf_size, sb->inode_table_blocks) * BLOCK_SIZE;

    DEBUGP("malloc size %d\n", buf_size);

    u8 *buf = malloc(buf_size);
    if (buf == NULL)
    {
        panic("Cannot allocate memory!!!");
    }
    memset(buf, 0, buf_size);

    DEBUGP("buffer %X\n", buf);

    /* 初始化块位图block_bitmap */
    buf[ROOT_DIR_IDX] |= 0b11; // 第0块占位不做处理，第1个块预留给根目录,位图中先占位
                               // 由于 0 表示不存在，所以这里就这样处理了
                               // 尽管可以利用其他机制来利用这块区域
                               // 但是以文件系统的尿性，不稀罕！！！ 哈哈哈哈哈
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
    partition_write(part, sb->block_bitmap_lba, buf, sb->block_bitmap_blocks * BLOCK_SECTOR_COUNT);

    /*3 将inode位图初始化并写入sb->inode_bitmap_lba */

    memset(buf, 0, buf_size);
    buf[ROOT_DIR_IDX] |= 0x1; // 第0个inode分给了根目录
    partition_write(part, sb->inode_bitmap_lba, buf, sb->inode_bitmap_blocks * BLOCK_SECTOR_COUNT);

    /* 4 将inode数组初始化并写入sb->inode_table_lba */

    /* 准备写inode_table中的第0项,即根目录所在的inode */
    memset(buf, 0, buf_size); // 先清空缓冲区buf
    Inode *i = buf;
    i->size = sb->dir_entry_size * 2; // .和..
    i->nr = 0;                        // 根目录占inode数组中第0个inode
    i->blocks[0] = 1;                 // 由于上面的memset,i_sectors数组的其它元素都初始化为0
    partition_write(part, sb->inode_table_lba, buf, sb->inode_table_blocks * BLOCK_SECTOR_COUNT);

    /* 5 将根目录初始化并写入sb->data_start_lba */
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
    partition_write(part, sb->data_start_lba + BLOCK_SECTOR_COUNT, buf, BLOCK_SECTOR_COUNT);
    // 写入第一块

    DEBUGP("%s format done\n", part->name);
    free(buf);
    part->fs = FS_ONIX;
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
    // PBMB;
    partition_read(part, BLOCK_SECTOR_COUNT, sb, 1);
    if (sb->magic == FS_MAGIC)
    {
        part->fs = FS_ONIX;
        DEBUGP("%s has file system\n", part->name);
        // partition_format(part);
    }
    else if (!strcmp(part->name, default_part)) // todo remove default part if need...
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
    while (idx < disk->primary_count)
    {

        search_part_fs(disk, part);
        part++;
        idx++;
    }
    part = disk->logical_parts;
    idx = 0;
    while (idx < disk->logical_count)
    {

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

extern void init_dir();

void init_fs()
{
    CHECK_STACK;
    extern u8 channel_count;
    u8 idx = 0;
    DEBUGK("searching filesystem in %d channel.....\n", channel_count);
    while (idx < channel_count)
    {
        IDEChannel *channel = channels + idx;
        search_channel_fs(channel);
        idx++;
    }
    queue_traversal(&partition_queue, partition_mount, default_part);
    init_dir();
    DEBUGK("Init file system finished.\n");
}

void *get_path_part(const char *pathname)
{
    return root_part; // todo for mounted part in the future
}
