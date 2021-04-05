#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/io.h>
#include <onix/kernel/clock.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/queue.h>
#include <onix/malloc.h>
#include <onix/syscall.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

u8 channel_count;
IDEChannel channels[2];

static u8 harddisk_count;

Queue partition_queue;

void harddisk_handler(int vector)
{
    DEBUGP("Harddisk interrupt occured!!!\n");

    assert(vector == 0x2e || vector == 0x2f);
    u8 idx = vector - 0x2e;
    IDEChannel *channel = &channels[idx];

    assert(channel->irq == vector);

    if (channel->waiting)
    {
        DEBUGP("harddisk sema up \n");
        channel->waiting = false;
        sema_up(&channel->done);
        inb(ATA_REG_STATUS(channel));
    }

    DEBUGP("Harddisk interrupt finish!!!\n");
}

void select_disk(Harddisk *disk)
{
    u8 reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
    if (disk->dev_idx == 1)
    {
        reg_device |= BIT_DEV_DEV;
    }
    outb(ATA_REG_DEVICE(disk->channel), reg_device);
}

void select_sector(Harddisk *disk, u32 lba, u32 sec_cnt)
{
    assert(lba <= MAX_LBA);
    IDEChannel *channel = disk->channel;
    outb(ATA_REG_NSECTOR(channel), sec_cnt);
    outb(ATA_REG_LBA_LOW(channel), lba);
    outb(ATA_REG_LBA_MID(channel), lba >> 8);
    outb(ATA_REG_LBA_HIGH(channel), lba >> 16);

    u8 reg_device = BIT_DEV_MBS | BIT_DEV_LBA | (lba >> 24);
    if (disk->dev_idx == 1)
    {
        reg_device |= BIT_DEV_DEV;
    }
    outb(ATA_REG_DEVICE(channel), reg_device);
}

void command_out(IDEChannel *channel, u8 cmd)
{
    // DEBUGP("harddisk command out \n");
    channel->waiting = true;
    outb(ATA_REG_CMD(channel), cmd);
}

void read_sectors(Harddisk *disk, void *buf, u8 sec_cnt)
{
    // 由于 sec_cnt 是 8 位，实际不可能读 0 个扇区，于是用于表示 256；
    u32 size = ((sec_cnt) ? sec_cnt : 256) * SECTOR_SIZE;
    insw(ATA_REG_DATA(disk->channel), buf, size / 2); // ata 数据端口 16 位
}

void write_sectors(Harddisk *disk, void *buf, u8 sec_cnt)
{
    u32 size = ((sec_cnt) ? sec_cnt : 256) * SECTOR_SIZE;
    outsw(ATA_REG_DATA(disk->channel), buf, size / 2);
}

bool harddisk_busy_wait(Harddisk *disk)
{
    IDEChannel *channel = disk->channel;
    u16 time_limit = 30 * 1000;
    u8 status;
    while (time_limit >= 0)
    {
        status = inb(ATA_REG_STATUS(channel));
        if (status & ATA_SR_BSY)
        {
            sys_sleep(10);
            continue;
        }
        return status & ATA_SR_DRQ;
    }
    printk("harddisk wait timeout\n");
    return false;
}

void harddisk_read(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{

#ifdef ONIX_KERNEL_DEBUG
    extern void harddisk_read(Harddisk * disk, u32 lba, void *buf, u32 sec_cnt);
    return debug_harddisk_read(disk, lba, buf, sec_cnt);
#endif

    DEBUGP("read disk %X lba %X buf %X sect %d \n", disk, lba, buf, sec_cnt);

    assert(lba < MAX_LBA);
    assert(sec_cnt > 0);

    DEBUGP("read disk acquire \n");

    acquire(&disk->channel->lock);

    DEBUGP("read disk select disk \n");

    select_disk(disk);

    u32 sec_op = 0;
    u32 sec_done = 0;
    // 尽量一次处理 256 个扇区
    while (sec_done < sec_cnt)
    {
        if ((sec_done + 256) <= sec_cnt)
        {
            sec_op = 256;
        }
        else
        {
            sec_op = sec_cnt - sec_done;
        }

        DEBUGP("read disk select sector \n");
        select_sector(disk, lba + sec_done, sec_op);

        DEBUGP("read disk command out \n");
        command_out(disk->channel, ATA_CMD_READ_PIO);

        DEBUGP("read disk sema down\n");
        sema_down(&disk->channel->done);

        if (!harddisk_busy_wait(disk))
        {
            panic("%s read sector %d failed!!!\n", disk->name, lba);
        }

        DEBUGP("read disk read sectors\n");
        read_sectors(disk, (void *)((u32)buf + sec_done * SECTOR_SIZE), sec_op);
        sec_done += sec_op;
    }
    release(&disk->channel->lock);
}

void harddisk_write(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{

#ifdef ONIX_KERNEL_DEBUG
    extern void debug_harddisk_write(Harddisk * disk, u32 lba, void *buf, u32 sec_cnt);
    return debug_harddisk_write(disk, lba, buf, sec_cnt);
#endif
    assert(lba < MAX_LBA);
    assert(sec_cnt > 0);
    DEBUGP("write disk acquire\n");
    acquire(&disk->channel->lock);

    DEBUGP("write disk select disk\n");
    select_disk(disk);

    u32 sec_op = 0;
    u32 sec_done = 0;
    while (sec_done < sec_cnt)
    {
        if ((sec_done + 256) <= sec_cnt)
        {
            sec_op = 256;
        }
        else
        {
            sec_op = sec_cnt - sec_done;
        }

        DEBUGP("write disk select sector\n");
        select_sector(disk, lba + sec_done, sec_op);

        DEBUGP("write disk command out \n");
        command_out(disk->channel, ATA_CMD_WRITE_PIO);

        if (!harddisk_busy_wait(disk))
        {
            panic("%s write sector %d failed!!!\n", disk->name, lba);
        }

        DEBUGP("write disk write sectors\n");
        write_sectors(disk, (void *)((u32)buf + sec_done * SECTOR_SIZE), sec_op);

        DEBUGP("write disk sema down\n");
        sema_down(&disk->channel->done);
        sec_done += sec_op;
    }
    release(&disk->channel->lock);
}

void partition_read(Partition *part, u32 lba, void *buf, u32 sec_cnt)
{
    assert(lba + sec_cnt <= part->sec_cnt);
    Harddisk *disk = part->disk;
    harddisk_read(disk, part->start_lba + lba, buf, sec_cnt);
}

void partition_write(Partition *part, u32 lba, void *buf, u32 sec_cnt)
{
    assert(lba + sec_cnt <= part->sec_cnt);
    Harddisk *disk = part->disk;
    harddisk_write(disk, part->start_lba + lba, buf, sec_cnt);
    DEBUGK("part 0x%X lba 0x%X\n", part, (part->start_lba + lba));
}

static void swap_pairs_bytes(const char *dst, char *buf, u32 len)
{
    u32 i = 0;
    for (i = 0; i < len; i += 2)
    {
        buf[i + 1] = *dst++;
        buf[i] = *dst++;
    }
    buf[i] = '\0';
}

static void disk_identity(Harddisk *disk)
{
    DEBUGP("disk indentity start \n");

    char info[SECTOR_SIZE];

    DEBUGP("disk indentity select disk \n");
    select_disk(disk);

    DEBUGP("disk indentity send command \n");
    command_out(disk->channel, ATA_CMD_IDENTIFY);

    DEBUGP("disk indentity sema down \n");
    sema_down(&disk->channel->done);

    if (!harddisk_busy_wait(disk))
    {
        panic("%s identity failed!!!\n", disk->name);
    }

    DEBUGP("disk indentity read sector \n");
    read_sectors(disk, info, 1);

    DEBUGP("disk indentity  read finish \n");
    char buf[64];

    u8 sn_start = 10 * 20;
    u8 sn_len = 20;
    u8 md_start = 27 * 2;
    u8 md_len = 40;

    swap_pairs_bytes(&info[sn_start], buf, sn_len);
    DEBUGP("Disk %s info: \n", disk->name);
    DEBUGP("     SN: %s \n", buf);

    memset(buf, 0, sizeof(buf));

    swap_pairs_bytes(&info[md_start], buf, md_len);
    DEBUGP("    Module: %s \n", buf);

    u32 sectors = *(u32 *)&info[60 * 2];
    DEBUGP("    Sectors: %d\n", sectors);
    DEBUGP("    Capacity: %dMB\n", sectors * SECTOR_SIZE / 1024 / 1024);
}

static void partition_scan(Harddisk *disk, u32 start_lba, u32 ext_lba)
{
    DEBUGP("part scan start \n");
    BootSector holder;
    BootSector *bs = &holder;

    Task *task = running_task();
    assert(task->magic == TASK_MAGIC);

    harddisk_read(disk, start_lba, bs, 1);

    PartitionTableEntry *entry = bs->entry;

    for (u32 part_idx = 0; part_idx < 4; part_idx++, entry++)
    {
        // DEBUGP("Part idx %d fs %d entry 0x%X\n", part_idx, entry->fs_type, entry);
        if (entry->fs_type == FS_UNKNOWN)
            continue;
        if (entry->fs_type == FS_EXTEND)
        {
            if (ext_lba != 0)
            {
                partition_scan(disk, entry->start_lba + ext_lba, ext_lba);
            }
            else
            {
                partition_scan(disk, entry->start_lba, entry->start_lba);
            }
            continue;
        }
        if (ext_lba == 0)
        {
            assert(disk->primary_count < MAX_PRIMARY_PART);

            Partition *part = disk->primary_parts + disk->primary_count;

            part->start_lba = ext_lba + entry->start_lba;
            part->sec_cnt = entry->sec_cnt;
            part->disk = disk;
            part->type = PART_PRIMARY;
            DEBUGP("part scan push primary queue \n");
            queue_push(&partition_queue, &part->node);
            sprintf(part->name, "%s%d", disk->name, disk->primary_count + 1);
            disk->primary_count++;
        }
        else
        {
            if (disk->logical_count >= MAX_LOGICAL_PART)
                continue;

            Partition *part = disk->logical_parts + disk->logical_count;

            part->start_lba = ext_lba + entry->start_lba;
            part->sec_cnt = entry->sec_cnt;
            part->disk = disk;
            part->type = PART_LOGICAL;
            DEBUGP("part scan push primary queue\n");
            queue_push(&partition_queue, &part->node);

            sprintf(part->name, "%s%d", disk->name, disk->logical_count + disk->primary_count + 1);
            disk->logical_count++;
        }
    };
}

static bool print_partition_info(Node *node, int arg)
{
    Partition *part = element_entry(Partition, node, node);
    DEBUGP("%s type %d start_lba:0x%x, sec_cnt:0x%x\n",
           part->name, part->type,
           part->start_lba, part->sec_cnt);
    return false;
}

static void init_disk(IDEChannel *channel, u8 dev_idx)
{
    Harddisk *disk = channel->devices + dev_idx;

    disk->channel = channel;
    disk->dev_idx = dev_idx;

    sprintf(disk->name, "sd%c", 'a' + channel->index * 2 + disk->dev_idx);
    disk->logical_count = 0;
    disk->primary_count = 0;
    if (disk->dev_idx != 0) // skip master.img
    {
        DEBUGP("init partition scan %s\n", disk->name);
        partition_scan(disk, 0, 0);
    }
}

static u32 get_harddisk_count()
{
#ifdef ONIX_KERNEL_DEBUG
    return 2;
#else
    return *((u8 *)(0x475));
#endif
}

static void init_channels()
{
    harddisk_count = get_harddisk_count();
    assert(harddisk_count > 0);
    queue_init(&partition_queue);

    channel_count = round_up(harddisk_count, 2);

    DEBUGP("harddisk count %d channel count %d\n", harddisk_count, channel_count);

    IDEChannel *channel;
    u32 channel_idx = 0;
    u8 dev_idx = 0;

    while (channel_idx < channel_count)
    {
        channel = channels + channel_idx;
        channel->index = channel_idx;
        sprintf(channel->name, "ide%u\0", 0);
        DEBUGP("channel name %s\n", channel->name);

        switch (channel_idx)
        {
        case 0:
            channel->bus = ATA_BUS_PRIMARY;
            channel->irq = ICW2_INT_VECTOR_IRQ0 + IRQ_HARDDISK;
            break;
        case 1:
            channel->bus = ATA_BUS_SECONDARY;
            channel->irq = ICW2_INT_VECTOR_IRQ0 + IRQ_HARDDISK2;
        default:
            break;
        }

        channel->waiting = false;
        lock_init(&channel->lock);
        sema_init(&channel->done, 0);

        while (dev_idx < 2)
        {
            init_disk(channel, dev_idx);
            dev_idx++;
        }
        dev_idx = 0;
        channel_idx++;
    }
    DEBUGP("init show partition info\n");
    queue_traversal(&partition_queue, print_partition_info, NULL);
}

void init_harddisk()
{
    printk("Initializing harddisk...\n");

    DEBUGP("harddisk enable irq\n");
    enable_irq(IRQ_CASCADE);
    enable_irq(IRQ_HARDDISK);

    register_handler(IRQ_HARDDISK, harddisk_handler);
    register_handler(IRQ_HARDDISK2, harddisk_handler);

    init_channels();
    printk("Initializing harddisk finish...\n");
}