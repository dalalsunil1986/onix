#include <onix/kernel/interrupt.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/clock.h>
#include <onix/kernel/keyboard.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

#define EFLAGS_IF 0x00000200

InterruptGate idt[IDT_SIZE];
Pointer idt_ptr;
InterruptHandler handler_table[IDT_SIZE];

extern u32 get_eflags();

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
        InterruptGate *gate = &idt[i];
        InterruptHandler handler = interrupt_entry_table[i];

        gate->offset0 = (u32)handler & 0xffff;
        gate->offset1 = ((u32)handler & 0xffff0000) >> 16;
        gate->selector = 8;
        gate->present = 1;
        gate->type = 0b1110;
        gate->DPL = PL0;
    }
    idt_ptr.base = (u32)&idt;
    idt_ptr.limit = sizeof(idt) - 1;
    load_idt(&idt_ptr);
}

static void default_handler(int vector)
{
    printk("default interrupt handler 0x%X \n", vector);
}

static void exception_handler(int vector, int code, int eip, int cs, int eflags)
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

    printk("Exception: %s \n EFLAGS: 0x%X CS: 0x%X EIP: 0x%X \n ErrorCode: 0x%X \n",
           messages[vector],
           eflags,
           cs,
           eip,
           code);
    halt();
}

void init_handler()
{
    for (size_t i = 20; i < IDT_SIZE; i++)
    {
        handler_table[i] = &default_handler;
    }
}

void init_exception()
{
    for (size_t i = 0; i < 20; i++)
    {
        handler_table[i] = &exception_handler;
    }
}

static void test_interrupt()
{
    DEBUGK("Test interrupt...\n");
    // u32 *addr = (u32 *)0x22222222;
    // *addr = 0x22222222;
}

void init_interrupt()
{
    printk("Initializing interrupt...\n");
    init_pit();
    init_pic();
    init_idt();
    init_exception();
    init_handler();
    init_clock();
    init_keyboard();
    test_interrupt();
    printk("Initializing interrupt finished...\n");
}

void register_handler(u32 irq, InterruptHandler handler)
{
    handler_table[ICW2_INT_VECTOR_IRQ0 + irq] = handler;
}

bool get_interrupt_status()
{
    u32 eflag = get_eflags();
    return (eflag & EFLAGS_IF) ? true : false;
}