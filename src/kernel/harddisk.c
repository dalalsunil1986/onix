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

static u8 harddisk_count;

u8 channel_count;
IDEChannel channels[2];

u32 ext_lba_base = 0;
u8 p_no = 0;
u8 l_no = 0;

Queue partition_queue;

void harddisk_handler(int vector)
{
    DEBUGP("Harddisk interrupt occured!!!\n");
    assert(vector == 0x2e || vector == 0x2f);
    u8 idx = vector - 0x2e;
    IDEChannel *channel = &channels[idx];
    assert(channel->irq_no == vector);

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
    if (disk->dev_no == 1)
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

    u8 reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
    if (disk->dev_no == 1)
    {
        reg_device |= BIT_DEV_DEV;
    }
    else
    {
        reg_device |= (lba >> 24);
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
    insd(ATA_REG_DATA(disk->channel), buf, size / 4);
}

void write_sectors(Harddisk *disk, void *buf, u8 sec_cnt)
{
    u32 size = ((sec_cnt) ? sec_cnt : 256) * SECTOR_SIZE;
    outsd(ATA_REG_DATA(disk->channel), buf, size / 4);
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
        }
        else
        {
            return status & ATA_SR_DRQ;
        }
        /* code */
    }
    printk("harddisk wait timeout \n");
    return false;
}

void harddisk_read(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{
    DEBUGP("harddisk read disk %X lba %X buf %X sect %d \n", disk, lba, buf, sec_cnt);

    assert(lba < MAX_LBA);
    assert(sec_cnt > 0);

    DEBUGP("harddisk read disk acquire \n");

    acquire(&disk->channel->lock);

    DEBUGP("harddisk read disk select disk \n");

    select_disk(disk);

    u32 secs_op;
    u32 secs_done;
    while (secs_done < sec_cnt)
    {
        if ((secs_done + 256) <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }
        DEBUGP("harddisk read disk select sector \n");
        select_sector(disk, lba + secs_done, secs_op);

        DEBUGP("harddisk read disk command out \n");
        command_out(disk->channel, ATA_CMD_READ_PIO);

        DEBUGP("harddisk read disk sema down\n");
        // DEBUGP("");

        sema_down(&disk->channel->done);

        if (!harddisk_busy_wait(disk))
        {
            panic("%s read sector %d failed!!!\n", disk->name, lba);
        }

        DEBUGP("harddisk read disk read sectors \n");
        read_sectors(disk, (void *)((u32)buf + secs_done * SECTOR_SIZE), secs_op);
        secs_done += secs_op;
    }
    release(&disk->channel->lock);
}

void harddisk_write(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{
    assert(lba < MAX_LBA);
    assert(sec_cnt > 0);

    acquire(&disk->channel->lock);
    select_disk(disk);

    u32 secs_op;
    u32 secs_done;
    while (secs_done < sec_cnt)
    {
        if ((secs_done + 256) <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }
        select_sector(disk, lba + secs_done, secs_op);

        DEBUGP("harddisk write disk command out \n");
        command_out(disk->channel, ATA_CMD_WRITE_PIO);

        if (!harddisk_busy_wait(disk))
        {
            panic("%s write sector %d failed!!!\n", disk->name, lba);
        }
        write_sectors(disk, (void *)((u32)buf + secs_done * SECTOR_SIZE), secs_op);

        DEBUGP("harddisk write disk sema down\n");
        sema_down(&disk->channel->done);
        secs_done += secs_op;
    }
    release(&disk->channel->lock);
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
    DEBUGP("Disk %s info: \n    SN: %s \n", disk->name, buf);

    memset(buf, 0, sizeof(buf));

    swap_pairs_bytes(&info[md_start], buf, md_len);
    DEBUGP("    Module: %s \n", buf);

    u32 sectors = *(u32 *)&info[60 * 2];
    DEBUGP("    Sectors: %d\n", sectors);
    DEBUGP("    Capacity: %dMB\n", sectors * SECTOR_SIZE / 1024 / 1024);
}

static void partition_scan(Harddisk *disk, u32 ext_lba)
{
    // BMB;

    DEBUGP("part scan start \n");

    BootSector *bs = malloc(sizeof(BootSector));

    DEBUGP("part scan memory alloc 0x%08X\n", bs);

    harddisk_read(disk, ext_lba, bs, 1);

    DEBUGP("part scan read bs finish\n");
    u8 part_idx = 0;
    PartitionTableEntry *entry = bs->entry;

    for (size_t part_idx = 0; part_idx < 4; part_idx++, entry++)
    {
        DEBUGP("Part idx %d fs %d entry 0x%X\n", part_idx, entry->fs_type, entry);
        if (entry->fs_type == 0)
            continue;
        if (entry->fs_type == 0x5)
        {
            if (ext_lba_base != 0)
            {
                partition_scan(disk, entry->start_lba + ext_lba_base);
            }
            else
            {
                ext_lba_base = entry->start_lba;
                partition_scan(disk, entry->start_lba);
            }
        }
        else if (entry->fs_type != 0)
        {
            if (ext_lba_base == 0)
            {
                disk->primary_parts[p_no].start_lba = ext_lba + entry->start_lba;
                disk->primary_parts[p_no].sec_cnt = entry->sec_cnt;
                disk->primary_parts[p_no].disk = disk;
                DEBUGP("part scan push primary queue \n");
                queue_push(&partition_queue, &disk->primary_parts[p_no].node);
                sprintf(disk->primary_parts[p_no].name, "%s%d", disk->name, p_no + 1);
                p_no++;
                assert(p_no < 4);
            }
            else
            {
                disk->primary_parts[l_no].start_lba = ext_lba + entry->start_lba;
                disk->primary_parts[l_no].sec_cnt = entry->sec_cnt;
                disk->primary_parts[l_no].disk = disk;
                DEBUGP("part scan push logical queue \n");
                queue_push(&partition_queue, &disk->logical_parts[l_no].node);
                sprintf(disk->primary_parts[p_no].name, "%s%d", disk->name, l_no + 5);
                l_no++;
                if (l_no >= 8)
                    return;
            }
        }
    };
    free(bs);
}

static bool print_partition_info(Node *node, int arg)
{
    Partition *part = element_entry(Partition, node, node);
    printk("   %s start_lba:0x%x, sec_cnt:0x%x\n", part->name, part->start_lba, part->sec_cnt);
    return false;
}

void init_harddisk()
{
    printk("Initializing harddisk...\n");
    harddisk_count = *((u8 *)(0x475));

    queue_init(&partition_queue);

    channel_count = round_up(harddisk_count, 2);

    DEBUGP("harddisk count %d channel count %d\n", harddisk_count, channel_count);
    IDEChannel *channel;
    u32 channel_no = 0;
    u8 dev_no = 0;

    while (channel_no < channel_count)
    {
        channel = channels + channel_no;
        sprintf(channel->name, "ide%u\0", 0);
        DEBUGP("name %s\n", channel->name);
        switch (channel_no)
        {
        case 0:
            channel->bus = ATA_BUS_PRIMARY;
            channel->irq_no = ICW2_INT_VECTOR_IRQ0 + IRQ_HARDDISK;
            break;
        case 1:
            channel->bus = ATA_BUS_SECONDARY;
            channel->irq_no = ICW2_INT_VECTOR_IRQ0 + IRQ_HARDDISK2;
        default:
            break;
        }
        channel->waiting = false;
        lock_init(&channel->lock);
        sema_init(&channel->done, 0);

        register_handler(IRQ_HARDDISK, harddisk_handler);
        register_handler(IRQ_HARDDISK2, harddisk_handler);

        DEBUGP("harddisk enable irq\n");
        enable_irq(IRQ_CASCADE);
        enable_irq(IRQ_HARDDISK);

        while (dev_no < 2)
        {
            Harddisk *disk = &channel->devices[dev_no];
            disk->channel = channel;
            disk->dev_no = dev_no;
            sprintf(disk->name, "sd%c", 'a' + channel_no * 2 + dev_no);
            disk_identity(disk);
            if (dev_no != 0)
            {
                DEBUGP("init partition scan %s\n", disk->name);
                partition_scan(disk, 0);
            }
            p_no = 0;
            l_no = 0;
            dev_no++;
        }
        dev_no = 0;
        channel_no++;
    }
    DEBUGP("init show partition info\n");
    queue_traversal(&partition_queue, print_partition_info, NULL);
}