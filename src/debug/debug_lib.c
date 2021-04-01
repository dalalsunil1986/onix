
#include <onix/types.h>

// 伪造底层汇编函数，用于调试上层逻辑

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

void *running_task()
{
    return 0x10000;
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