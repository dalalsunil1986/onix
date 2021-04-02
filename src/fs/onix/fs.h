#ifndef ONIX_FS_H
#define ONIX_FS_H

#include <onix/types.h>
#include <onix/queue.h>

#define MAX_FILENAME_LENGTH 64
#define MAX_FILE_PER_PART 4096
#define SECTOR_SIZE 512
#define BLOCK_SECTOR_COUNT 1
#define BLOCK_SIZE (SECTOR_SIZE * BLOCK_SECTOR_COUNT)
#define BLOCK_BITS (BLOCK_SIZE * 8)
#define FS_MAGIC 0x20210330
#define DIRECT_BLOCK_CNT 12
#define INDIRECT_BLOCK_CNT 1
#define INODE_BLOCK_CNT (DIRECT_BLOCK_CNT + INDIRECT_BLOCK_CNT)

#define ROOT_DIR_IDX 0

typedef enum FileType
{
    FILETYPE_UNKNOWN,
    FILETYPE_REGULAR,
    FILETYPE_DIRECTORY,
} FileType;

typedef enum BitmapType
{
    INODE_BITMAP,
    BLOCK_BITMAP
} BitmapType;

typedef struct SuperBlock
{
    u32 magic;
    u32 sec_cnt;   // 分区扇区数量
    u32 inode_cnt; // 分区 inode 数量
    u32 start_lba; // 分区起始 lba 地址

    u32 block_bitmap_lba;    // 块位图起始 lba 地址
    u32 block_bitmap_blocks; // 块位图占用的块数量

    u32 inode_bitmap_lba;    // inode 位图起始 lba 地址
    u32 inode_bitmap_blocks; // inode 位图占用的块数量

    u32 inode_table_lba;    // inode 表起始 lba 地址
    u32 inode_table_blocks; // inode 表占用的块数量

    u32 data_start_lba; // 数据区开始的 lba 地址
    u32 root_inode_nr;  // 根目录所在的 inode 号
    u32 dir_entry_size; // 目录项大小
    u8 placeholder[460];
} _packed SuperBlock;

typedef struct Inode
{
    /* 文件内容描述符 
    用来索引、跟踪一个文件的所有块，
    文件的元信息存储在此
    */
    u32 nr;                      // inode 号
    u32 size;                    // 文件或目录大小
    u32 open_cnts;               // 文件被打开的次数
    bool write_deny;             // 写文件标志
    u32 blocks[INODE_BLOCK_CNT]; // 0-11 直接快 12 一级间接块
    Node node;
} Inode;

typedef struct Dir
{
    Inode *inode;
    u32 dir_offset;         // 记录在目录内的偏移
    u32 buffer[BLOCK_SIZE]; // 目录的数据缓存
} Dir;

typedef struct DirEntry
{
    /*
    文件入口描述符
    */
    char filename[MAX_FILENAME_LENGTH];
    u32 inode_nr; // 对应的 inode 号
    FileType type;
} DirEntry;

void init_fs();

#endif