#ifndef ONIX_GLOBAL_H
#define ONIX_GLOBAL_H

#include <onix/types.h>

#define GDT_SIZE 128
#define IDT_SIZE 256

#define PL0 0b0
#define PL1 0b1
#define PL2 0b10
#define PL3 0b11
#define TI_GDT 0
#define TI_LDT 1

/* 存储段描述符/系统段描述符 */
typedef struct Descriptor /* 共 8 个字节 */
{
    u16 limit_low;     /* Limit 0 - 15 bit */
    u32 base_low : 24; /* Base 0 - 15 */
    // u8 accessed : 1;       // is accessed for cpu
    // u8 read_write : 1;     // readable for code, writable for data
    // u8 conform_expand : 1; //  conform for code, expand down for data
    // u8 code : 1;           // 1 for code, 2 for data
    u8 type : 4;
    u8 segment : 1;     // 1 for everything but TSS and LDT
    u8 DPL : 2;         // Descriptor Privilege Level
    u8 present : 1;     // Present bit, is in memory or disk
    u8 limit_high : 4;  // limit 16-19;
    u8 available : 1;   // Available Everything is settledown, but 1 bit not in use
    u8 long_mode : 1;   // 64 bit extend bit;
    u8 big : 1;         // 32 bit or 16 bit;
    u8 granularity : 1; // granularity; 4KB or 1B
    u8 base_high;       /* Base */
} _packed Descriptor;

typedef struct Selector
{
    u8 RPL : 2;
    u8 TI : 1;
    u16 index : 13;
} Selector;

typedef struct Pointer
{
    u16 limit;
    u32 base;
} _packed Pointer;

extern Pointer gdt_ptr;
extern Descriptor gdt[GDT_SIZE];

static const int SELECT_KERNEL_CODE_INDEX = 1;
static const int SELECT_KERNEL_DATA_INDEX = 2;
static const int SELECT_KERNEL_VIDEO_INDEX = 3;
static const int SELECT_KERNEL_TSS_INDEX = 4;

static const Selector SELECTOR_KERNEL_CODE = {PL0, TI_GDT, SELECT_KERNEL_CODE_INDEX};
static const Selector SELECTOR_KERNEL_DATA = {PL0, TI_GDT, SELECT_KERNEL_DATA_INDEX};
static const Selector SELECTOR_KERNEL_VIDEO = {PL0, TI_GDT, SELECT_KERNEL_VIDEO_INDEX};
static const Selector SELECTOR_KERNEL_TSS = {PL0, TI_GDT, SELECT_KERNEL_TSS_INDEX};

extern void load_gdt(Pointer *gdt_ptr);
extern void load_idt(Pointer *idt_ptr);
extern void save_gdt(Pointer *gdt_ptr);
extern void load_tss(Selector *tss_selector);

void init_descriptor(Descriptor *desc, u32 base, u32 limit);

void init_gdt();
#endif