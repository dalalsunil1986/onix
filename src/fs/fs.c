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

static void partition_format(Partition *part)
{
    u32 boot_sector_sects = 1;
    u32 superblock_sects = 1;
    u32 inode_bitmap_sects = round_up(MAX_FILE_PER_PART, BITS_PER_SECTOR);
    u32 inode_table_sects = round_up((sizeof(Inode) * MAX_FILE_PER_PART), SECTOR_SIZE);

    u32 used_sects = boot_sector_sects + superblock_sects + inode_bitmap_sects + inode_table_sects;
    u32 free_sects = part->sec_cnt - used_sects;

    u32 block_bitmap_sects = round_up(free_sects, BITS_PER_SECTOR);
    u32 block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = round_up(block_bitmap_bit_len, BITS_PER_SECTOR);

    SuperBlock sb;
    sb.magic = FS_MAGIC;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILE_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2;
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_lba;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode = 0;
    sb.dir_entry_size = sizeof(DirEntry);

    printk("%s info:\n", part->name);
    printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   data_start_lba:0x%x\n",
           sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt, sb.block_bitmap_lba,
           sb.block_bitmap_sects, sb.inode_bitmap_lba, sb.inode_bitmap_sects,
           sb.inode_table_lba, sb.inode_table_sects, sb.data_start_lba);

    Harddisk *disk = part->disk;
    harddisk_write(disk, part->start_lba + 1, &sb, 1);
    printk("   super_block_lba:0x%x\n", part->start_lba + 1);

    u32 buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? sb.block_bitmap_sects : sb.inode_bitmap_sects);
    buf_size = (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) * SECTOR_SIZE;

    DEBUGP("malloc size %d\n", buf_size);

    u8 *buf = malloc(buf_size);
    if (buf == NULL)
    {
        panic("Cannot allocate memory!!!");
    }

    DEBUGP("buffer %X\n", buf);

    /* 初始化块位图block_bitmap */
    buf[0] |= 0x01; // 第0个块预留给根目录,位图中先占位
    u32 block_bitmap_last_byte = block_bitmap_bit_len / 8;
    u8 block_bitmap_last_bit = block_bitmap_bit_len % 8;
    u32 last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
    // last_size是位图所在最后一个扇区中，不足一扇区的其余部分

    /* 1 先将位图最后一字节到其所在的扇区的结束全置为1,即超出实际块数的部分直接置为已占用*/
    memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    /* 2 再将上一步中覆盖的最后一字节内的有效位重新置0 */
    u8 bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit)
    {
        buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
    }
    harddisk_write(disk, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    // 3 将inode位图初始化并写入sb.inode_bitmap_lba *

    /* 先清空缓冲区*/
    memset(buf, 0, buf_size);
    buf[0] |= 0x1; // 第0个inode分给了根目录
    /* 由于inode_table中共4096个inode,位图inode_bitmap正好占用1扇区,
    * 即inode_bitmap_sects等于1, 所以位图中的位全都代表inode_table中的inode,
    * 无须再像block_bitmap那样单独处理最后一扇区的剩余部分,
    * inode_bitmap所在的扇区中没有多余的无效位 */
    harddisk_write(disk, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    // 4 将inode数组初始化并写入sb.inode_table_lba

    /* 准备写inode_table中的第0项,即根目录所在的inode */
    memset(buf, 0, buf_size); // 先清空缓冲区buf
    Inode *i = buf;
    i->size = sb.dir_entry_size * 2;   // .和..
    i->inode = 0;                      // 根目录占inode数组中第0个inode
    i->sectors[0] = sb.data_start_lba; // 由于上面的memset,i_sectors数组的其它元素都初始化为0
    harddisk_write(disk, sb.inode_table_lba, buf, sb.inode_table_sects);

    // 5 将根目录初始化并写入sb.data_start_lba
    /* 写入根目录的两个目录项.和.. */
    memset(buf, 0, buf_size);
    DirEntry *entry = buf;

    memcpy(entry->filename, ".", 1);
    entry->inode = 0;
    entry->type = FILETYPE_DIRECTORY;
    entry++;

    memcpy(entry->filename, "..", 2);
    entry->inode = 0; // 根目录的父目录依然是根目录自己
    entry->type = FILETYPE_DIRECTORY;
    harddisk_write(disk, sb.data_start_lba, buf, 1);

    printk("   root_dir_lba:0x%x\n", sb.data_start_lba);
    printk("%s format done\n", part->name);
    free(buf);
}

void init_fs()
{
    u8 channel_idx = 0;
    u8 dev_idx = 0;
    u8 part_idx = 0;

    SuperBlock *sb = malloc(SECTOR_SIZE);
    if (sb == NULL)
    {
        panic("alloc memory failed!!!");
    }

    DEBUGP("searching filesystem in %d channel.....\n", channel_count);

    while (channel_idx < channel_count)
    {
        dev_idx = 1;
        while (dev_idx < 2)
        {
            Harddisk *disk = &channels[channel_idx].devices[dev_idx];
            DEBUGP("search disk %s \n", disk->name);
            Partition *part = disk->primary_parts;
            while (part_idx < 12)
            {
                if (part_idx == 4)
                {
                    part = disk->logical_parts;
                }
                if (part->sec_cnt > 0)
                {
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
                }
                part_idx++;
                part++;
            }
            dev_idx++;
        }
        channel_idx++;
    }
    free(sb);
}