/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>
#include <onix/bitmap.h>

#define PAGE_SIZE 4096 // 4KB
#define ENTRY_SIZE (PAGE_SIZE / 4)

typedef struct
{
    u64 base;
    u64 size;
    u32 type;
} _packed ards_t;

typedef struct
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
} vaddr_t;

void init_ards(ards_t *ards, u32 count);
void init_memory();

typedef page_entry_t (*page_table_t)[ENTRY_SIZE];

extern u32 memory_base;
extern u32 memory_size;
extern u32 free_pages;
extern u32 total_pages;

page_table_t get_pde();
void set_pde(page_table_t pde);

#endif