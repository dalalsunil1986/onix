/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#ifndef ONIX_GLOBAL_H
#define ONIX_GLOBAL_H

#include <onix/types.h>

#define GDT_SIZE 128

#define PL0 0b0
#define PL1 0b1
#define PL2 0b10
#define PL3 0b11
#define TI_GDT 0b000
#define TI_LDT 0b100

/* 存储段描述符/系统段描述符 */
typedef struct /* 共 8 个字节 */
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
} _packed descriptor_t;

typedef struct
{
    u8 RPL : 2;
    u8 TI : 1;
    u16 index : 13;
} selector_t;

typedef struct
{
    u16 limit;
    u32 base;
} _packed pointer_t;

extern pointer_t gdt_ptr;
extern descriptor_t gdt[GDT_SIZE];

void init_gdt();

#endif