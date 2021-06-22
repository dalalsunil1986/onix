/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/debug.h>

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;
handler_t handler_table[IDT_SIZE];

static void init_pic()
{
    /* 初始化主片 */
    outb(PIC_M_CTRL, ICW1_INIT | ICW1_ICW4); // ICW1: 边沿触发,级联8259, 需要ICW4.
    outb(PIC_M_DATA, ICW2_INT_VECTOR_IRQ0);  // ICW2: 起始中断向量号为0x20,也就是IR[0-7] 为 0x20 ~ 0x27.
    outb(PIC_M_DATA, 0b00000100);            // ICW3: IR2接从片.
    outb(PIC_M_DATA, ICW4_8086);             // ICW4: 8086模式, 正常EOI

    /* 初始化从片 */
    outb(PIC_S_CTRL, ICW1_INIT | ICW1_ICW4); // ICW1: 边沿触发,级联8259, 需要ICW4.
    outb(PIC_S_DATA, ICW2_INT_VECTOR_IRQ8);  // ICW2: 起始中断向量号为0x28,也就是IR[8-15] 为 0x28 ~ 0x2F.
    outb(PIC_S_DATA, 2);                     // ICW3: 设置从片连接到主片的IR2引脚
    outb(PIC_S_DATA, ICW4_8086);             // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111110);
    outb(PIC_S_DATA, 0b11111111);
}

static void init_idt()
{
    for (size_t i = 0; i < IDT_SIZE; i++)
    {
        gate_t *gate = &idt[i];
        handler_t handler = entry_table[i];

        gate->offset0 = (u32)handler & 0xffff;
        gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
        gate->segment = 0;
        gate->selector = 8;
        gate->present = 1;
        gate->type = 0b1110;
        gate->DPL = PL0;
    }

    idt_ptr.base = (u32)&idt;
    idt_ptr.limit = sizeof(idt) - 1;

    asm volatile("lidt idt_ptr\n");
}

static void default_handler(int vector)
{
    printk("default interrupt 0x%X\n", vector);
}

static void exception_handler(
    int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    int code, int eip, int cs, int eflags)
{
    char *messages[] = {
        "#DE Divide Error\0",
        "#DB RESERVED\0",
        "--  NMI Interrupt\0",
        "#BP Breakpoint\0",
        "#OF Overflow\0",
        "#BR BOUND Range Exceeded\0",
        "#UD Invalid Opcode (Undefined Opcode)\0",
        "#NM Device Not Available (No Math Coprocessor)\0",
        "#DF Double Fault\0",
        "    Coprocessor Segment Overrun (reserved)\0",
        "#TS Invalid TSS\0",
        "#NP Segment Not Present\0",
        "#SS Stack-Segment Fault\0",
        "#GP General Protection\0",
        "#PF Page Fault\0",
        "--  (Intel reserved. Do not use.)\0",
        "#MF x87 FPU Floating-Point Error (Math Fault)\0",
        "#AC Alignment Check\0",
        "#MC Machine Check\0",
        "#XF SIMD Floating-Point Exception\0"};

    printk("Exception   : %s \n", messages[vector]);
    printk("    VECTOR  : 0x%02X\n", vector);
    printk("    CODE    : 0x%08X\n", code);
    printk("    EFLAGS  : 0x%08X\n", eflags);
    printk("    EIP     : 0x%08X\n", eip);
    printk("    CS      : 0x%02X\n", cs);

    while (true)
        ;
}

static void init_handler()
{
    for (size_t i = 20; i < IDT_SIZE; i++)
    {
        handler_table[i] = &default_handler;
    }
}

static void init_exception()
{
    for (size_t i = 0; i < 20; i++)
    {
        handler_table[i] = &exception_handler;
    }
}

bool set_interrupt(bool status)
{
    u32 value = EFLAGS_IF;
    asm volatile(
        "pushf\n"
        "cli\n"
        "pop %%eax\n"
        "and %%ebx, %%eax\n"
        : "=a"(value)
        : "b"(value));

    if (status)
    {
        asm volatile("sti\n");
    }
    return value > 0;
}

bool get_interrupt()
{
    u32 value = EFLAGS_IF;
    asm volatile(
        "pushf\n"
        "cli\n"
        "pop %%eax\n"
        "and %%ebx, %%eax\n"
        : "=a"(value)
        : "b"(value));

    if (value)
    {
        asm volatile("sti\n");
    }

    return value > 0;
}

void interrupt_end()
{
    outb(PIC_M_CTRL, PIC_EOI);
    outb(PIC_S_CTRL, PIC_EOI);
}

void init_interrupt()
{
    INFOK("Initializing interrupt...\n");
    init_exception();
    init_handler();
    init_pic();
    init_idt();
    set_interrupt(1);
}