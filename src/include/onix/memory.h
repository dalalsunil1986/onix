/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>
#include <onix/bitmap.h>

#define PG_P_0 0b0
#define PG_P_1 0b1
#define PG_RW_R 0b00
#define PG_RW_W 0b10
#define PG_US_S 0b000
#define PG_US_U 0b100

#define PAGE_SIZE 4096 // 4KB
#define ENTRY_SIZE (PAGE_SIZE / 4)

#define KERNEL_BASE_PTE 0x300
#define KERNEL_BASE_PAGE 0xC0100000 // 内核可用内存起始位置
#define KERNEL_ADDR_MASK 0xC0000000 // 内核虚拟地址起始位置
#define KERNEL_ADDR_SIZE 0x40000000 // 内核虚拟内存长度 1G

#define PDE_MASK 0xFFC00000

typedef struct
{
    u64 base;
    u64 size;
    u32 type;
} _packed ards_t;

typedef struct page_entry_t
{
    u8 present : 1;  // 1 present
    u8 write : 1;    // 0 read  / 1 write
    u8 user : 1;     // 1 user // 0 super user
    u8 pwt : 1;      // 1 page level write through
    u8 pcd : 1;      // 1 page level cache disable
    u8 accessed : 1; // 1 accessed
    u8 dirty : 1;    // 1 dirty
    u8 pat : 1;      // 1 page attribute
    u8 global : 1;   // 1 global page should be in tlb cache
    u8 avl : 3;      // not use by cpu
    u32 index : 20;  // index of page
} _packed page_entry_t;

typedef struct
{
    u32 start;
    bitmap_t mmap;
} addr_t;

void init_ards(ards_t *ards, u32 count);
void init_memory();

typedef page_entry_t (*page_table_t)[ENTRY_SIZE];

extern u32 memory_base;
extern u32 memory_size;
extern u32 free_pages;
extern u32 total_pages;

page_table_t get_cr3();
void set_cr3(page_table_t pde);

u32 get_paddr(u32 vaddr);
u32 kalloc_page(u32 size);
void kfree_page(u32 vaddr, u32 size);

#endif