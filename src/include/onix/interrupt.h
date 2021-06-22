/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include <onix/types.h>
#include <onix/global.h>

#define IDT_SIZE 0x100

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口

#define ICW1_ICW4 0x01      /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW2_INT_VECTOR_IRQ0 0x20
#define ICW2_INT_VECTOR_IRQ8 0x28

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define INT_VECTOR_DIVIDE 0x0
#define INT_VECTOR_DEBUG 0x1
#define INT_VECTOR_NMI 0x2
#define INT_VECTOR_BREAKPOINT 0x3
#define INT_VECTOR_OVERFLOW 0x4
#define INT_VECTOR_BOUNDS 0x5
#define INT_VECTOR_INVAL_OP 0x6
#define INT_VECTOR_COPROC_NOT 0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG 0x9
#define INT_VECTOR_INVAL_TSS 0xA
#define INT_VECTOR_SEG_NOT 0xB
#define INT_VECTOR_STACK_FAULT 0xC
#define INT_VECTOR_PROTECTION 0xD
#define INT_VECTOR_PAGE_FAULT 0xE
#define INT_VECTOR_COPROC_ERR 0x10

#define INT_VECTOR_SYSCALL 0x80

#define IRQ_CLOCK 0    // 时钟
#define IRQ_KEYBOARD 1 // 键盘
#define IRQ_CASCADE 2  // 8259 从片控制器
#define IRQ_SERIAL_2 3
#define IRQ_SERIAL_1 4
#define IRQ_PARALLEL_2 5

#define IRQ_FLOPPY 6 // 软盘控制器
#define IRQ_PARALLEL_1 7
#define IRQ_RTC 8
#define IRQ_REDIRECT 9

#define IRQ_MOUSE 12
#define IRQ_MATH 13
#define IRQ_HARDDISK 14
#define IRQ_HARDDISK2 15

#define PIC_EOI 0X20 /* End-of-interrupt command code */

typedef void(*handler_t);

typedef struct
{
    u16 offset0;
    u16 selector;
    u8 dcount;
    u8 type : 4;
    u8 segment : 1;
    u8 DPL : 2;
    u8 present : 1;
    u16 offset1;
} _packed gate_t;

extern gate_t idt[IDT_SIZE];
extern pointer_t idt_ptr;
extern handler_t entry_table[IDT_SIZE];
extern handler_t handler_table[IDT_SIZE];

void init_interrupt();
bool set_interrupt(bool status); // 设置中断允许位，返回设置之前的值
bool get_interrupt();            // 获取中断允许位

#endif