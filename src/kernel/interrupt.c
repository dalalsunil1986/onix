/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/io.h>
#include <onix/interrupt.h>
#include <onix/clock.h>
#include <onix/assert.h>
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

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
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

    if (vector == 0x0E) // page fault
    {
        u32 cr2 = 0;
        asm volatile("movl %%cr2, %%eax"
                     : "=a"(cr2));
        printk("    CR2     : 0x%08X\n", cr2);
    }

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

void set_request(u32 irq, bool enable)
{
    assert(irq >= 0 && irq < 16);
    u16 port;
    if (irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << irq));
    }
    else
    {
        outb(port, inb(port) | (1 << irq));
    }
}

void register_handler(u32 irq, handler_t handler)
{
    handler_table[ICW2_INT_VECTOR_IRQ0 + irq] = handler;
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
    init_pit();

    init_clock();

    set_interrupt(true);
}