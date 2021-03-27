#ifndef ONIX_HARDDISK_H
#define ONIX_HARDDISK_H

/* 
refer to https://wiki.osdev.org/PCI_IDE_Controller#Parallel.2FSerial_ATA.2FATAPI
*/

#include <onix/types.h>
#include <onix/queue.h>
#include <onix/bitmap.h>
#include <onix/kernel/mutex.h>

#define ATA_SR_BSY 0x80  // Busy
#define ATA_SR_DRDY 0x40 // Drive ready
#define ATA_SR_DF 0x20   // Drive write fault
#define ATA_SR_DSC 0x10  // Drive seek complete
#define ATA_SR_DRQ 0x08  // Data request ready
#define ATA_SR_CORR 0x04 // Corrected data
#define ATA_SR_IDX 0x02  // Index
#define ATA_SR_ERR 0x01  // Error

#define ATA_ER_BBK 0x80   // Bad block
#define ATA_ER_UNC 0x40   // Uncorrectable data
#define ATA_ER_MC 0x20    // Media changed
#define ATA_ER_IDNF 0x10  // ID mark not found
#define ATA_ER_MCR 0x08   // Media change request
#define ATA_ER_ABRT 0x04  // Command aborted
#define ATA_ER_TK0NF 0x02 // Track 0 not found
#define ATA_ER_AMNF 0x01  // No address mark

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_REG_DATA(channel) (channel->bus + 0)
#define ATA_REG_FEATURES(channel) (channel->bus + 1)
#define ATA_REG_ERROR(channel) ATA_REG_FEATURES(channel)
#define ATA_REG_NSECTOR(channel) (channel->bus + 2)
#define ATA_REG_LBA_LOW(channel) (channel->bus + 3)
#define ATA_REG_LBA_MID(channel) (channel->bus + 4)
#define ATA_REG_LBA_HIGH(channel) (channel->bus + 5)
#define ATA_REG_DEVICE(channel) (channel->bus + 6)
#define ATA_REG_STATUS(channel) (channel->bus + 7)
#define ATA_REG_CMD(channel) (ATA_REG_STATUS(channel))
#define ATA_REG_DCR(channel) (channel->bus + 0x206) /* device control register */
#define ATA_REG_CTL(channel) (ATA_REG_ALT(channel))

#define ATA_BUS_PRIMARY 0x1F0
#define ATA_BUS_SECONDARY 0x170

#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE 0xB0

#define BIT_DEV_MBS 0xA0
#define BIT_DEV_LBA 0x40
#define BIT_DEV_DEV 0x10

#define MAX_LBA ((16 * 1024 * 1024 / 512) - 1)

#define SECTOR_SIZE 512

typedef struct Partition
{
    u32 start_lba;
    u32 sec_cnt;
    struct Harddisk *disk;
    Node node;
    char name[8];
    struct SuperBlock *super_block;
    Bitmap block_bitmap;
    Bitmap inode_bitmap;
    Queue open_inodes;

} Partition;

typedef struct Harddisk
{
    char name[8];
    struct IDEChannel *channel;
    u8 dev_no;
    struct Partition primary_parts[4];
    struct Partition logical_parts[4];
} Harddisk;

typedef struct IDEChannel
{
    char name[8];
    u16 bus;
    u8 irq_no;
    Lock lock;
    bool waiting;
    Semaphore done;
    Harddisk devices[2];
} IDEChannel;

typedef struct PartitionTableEntry
{
    u8 bootable;
    u8 start_head;
    u8 start_sec;
    u8 start_chs;
    u8 fs_type;
    u8 end_head;
    u8 end_sec;
    u8 end_chs;
    u32 start_lba;
    u32 sec_cnt
} _packed PartitionTableEntry;

typedef struct BootSector
{
    u8 other[446];
    PartitionTableEntry entry[4];
    u16 signature;
} _packed BootSector;

extern u8 channel_count;
extern IDEChannel channels[];
void init_harddisk();

void harddisk_handler(int vector);

#endif