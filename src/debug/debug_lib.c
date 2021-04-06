
#include <onix/types.h>
#include <onix/kernel/harddisk.h>
#include <stdio.h>
#include <stdlib.h>
#include <onix/kernel/debug.h>
// 伪造底层汇编函数，用于调试上层逻辑

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

static u32 cr3;

void outb(u16 port, u8 value)
{
}
void outsw(u16 port, const void *addr, u32 size)
{
}
void outsd(u16 port, const void *addr, u32 size)
{
}
u8 inb(u16 port)
{
}
void insw(u16 port, const void *addr, u32 size)
{
}
void insd(u16 port, const void *addr, u32 size)
{
}
void halt()
{
    u32 counter = 10000;
    while (counter--)
        ;
}
void stop()
{
    while (1)
        ;
}
//  void pause(){

// }

u32 syscall0(u32 nr)
{
    return 0;
}
u32 syscall1(u32 nr, u32 arg)
{
    return 0;
}
u32 syscall2(u32 nr, u32 arg1, u32 arg2)
{
    return 0;
}
u32 syscall3(u32 nr, u32 arg1, u32 arg2, u32 arg3)
{
    return 0;
}

Task *task = NULL;

Task *running_task()
{
    if (task == NULL)
    {
        task = page_alloc(1);
    }
    task->magic = TASK_MAGIC;
    return task;
}

void switch_to(void *current, void *next)
{
    return;
}

bool enable_int()
{
    return 0;
}

bool disable_int()
{
    return 0;
}

u32 get_eflags()
{
    return 1;
}

void set_eflags(u32 eflags)
{
    return;
}

void set_cr3(u32 addr)
{
    cr3 = addr;
    return;
}

u32 get_cr3()
{
    return cr3;
}

void save_gdt()
{
}

void load_gdt()
{
}

void load_idt()
{
}

void load_tss()
{
}

void interrupt_exit()
{
}

u32 interrupt_entry_table[256];
u32 syscall_handler[256];

void debug_harddisk_read(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{
    FILE *file = fopen("slave.img", "rb+");
    fseek(file, lba * SECTOR_SIZE, 0);
    int count = fread(buf, SECTOR_SIZE, sec_cnt, file);
    // DEBUGP("read count %d\n", count);
    fclose(file);
    file = NULL;
}

void debug_harddisk_write(Harddisk *disk, u32 lba, void *buf, u32 sec_cnt)
{
    FILE *file = fopen("slave.img", "rb+");
    fseek(file, lba * SECTOR_SIZE, 0);
    int count = fwrite(buf, SECTOR_SIZE, sec_cnt, file);
    // DEBUGP("write count %d\n", count);
    fclose(file);
    file = NULL;
}