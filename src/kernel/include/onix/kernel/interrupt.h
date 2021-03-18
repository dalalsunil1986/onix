
/*
ICW1 FORMAT
0_0_0_1_LTIM_ADI_SINGLE_IC4
*/

// typedef struct ICW1
// {
//     u8 IC4 : 1;    // 1 需要 IC4 x86 必须为1
//     u8 SINGLE : 1; // 1 单片， 0 级联
//     u8 ADI : 1;    // 用来设置8085 的调用时间间隔， x86 不需要设置
//     u8 LTIM : 1;   // 0 边沿触发，1 电平触发
//     u8 LABEL : 4;  // == 1
// } ICW1;

/*
 ICW2 用来设置起始中断向量号
 ICW3 用来设置级联方式
    - 对于主片，ICW3 中置 1 的那一位对应的IRQ接口用于连接从片，若为0 则表示接外部设备。
*/

/*
ICW4 FORAMT
000_SFNM_BUF_MS_AEOI_PM
*/

// typedef struct ICW4
// {
//     u8 PM : 1;   // 处理器类型 0，则表示8080 或 8085 处理器，为1 ，则表示x86 处理器
//     u8 AOEI : 1; /* AEOI 表示自动结束中断 1 则表示自动结束中断，为0 ，则表示非自动，即手动结束中断，
//                     咱们可以在中断处理程序中或主函数中手动向8259A 的主、从片发送EOI 信号。 */
//     u8 MS : 1;   /* 当多个8259A 级联时，如果工作在缓冲模式下，
//                     MS 用来规定本8259A 是主片，还是从片。
//                     若 MS 为 1，则表示是主片，
//                     若 MS 为 0，则表示是从片。
//                     若工作在非缓冲模式 (BUF 为 0) 下， MS 无效。*/
//     u8 BUF : 1;  /* 表示本 8259A 芯片是否工作在缓冲模式。
//                     BUF 为 0，则工作非缓冲模式，
//                     BUF 为 1，则工作在缓冲模式。*/
//     u8 SFNM : 1; /* SFNM 表示特殊全嵌套模式 (Special Fully Nested Mode)，
//                     若SFNM 为 0，则表示全嵌套模式，
//                     若SFNM 为 1，则表示特殊全嵌套模式。*/
// } ICW4;

/*
OCW1 用来屏蔽连接在 8259A 上的外部设备的中断信号，
实际上就是把 OCW1 写入了 IMR 寄存器。
这里的屏蔽是说是否把来自外部设备的中断信号转发给CPU
*/

/*
OCW2 用来设置中断结束方式和优先级模式。
*/

/*
OCW3 用来设定特殊屏蔽方式及查询方式
*/


#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include <onix/types.h>
#include <onix/kernel/global.h>

#define IDT_SIZE 256

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

#define CLOCK_IRQ 0     // 时钟
#define KEYBOARD_IRQ 1  // 键盘
#define CASCADE_IRQ 2   // 8259 从片控制器
#define ETHER_IRQ 3     // COM2 / COM4
#define SECONDARY_IRQ 3 /* RS232 interrupt vector for port 2 */
#define RS232_IRQ 4     /* RS232 interrupt vector for port 1 */
#define XT_WINI_IRQ 5   /* xt winchester */
#define FLOPPY_IRQ 6    // 软盘控制器
#define PRINTER_IRQ 7
#define AT_WINI_IRQ 14 /* at winchester */

#define PIC_EOI 0X20 /* End-of-interrupt command code */

typedef void *InterruptHandler;

typedef struct InterruptGate
{
    u16 offset0;
    u16 selector;
    u8 dcount;
    u8 type : 4;
    u8 segment : 1;
    u8 DPL : 2;
    u8 present : 1;
    u16 offset1;
} _packed InterruptGate;

void init_interrupt();

extern InterruptGate idt[IDT_SIZE];
extern Pointer idt_ptr;
extern InterruptHandler interrupt_table[IDT_SIZE];

#endif