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
#define PG_SIZE 0x1000
#define PG_DIR_ADDRESS 0x1000
#define PG_BASE 0x100
#define BASE_ADDR_LIMIT 0x100000
#define KERNEL_ADDR_MASK 0xC0000000
#define KERNEL_MMAP_ADDR 0xC00003000
#define PREPARE_PMAP_ADDR 0xC00004000
#define KERNEL_PMAP_IDX 0xC0100
#define KERNEL_BASE_PAGE_VIDX 0xC0100

#define USER_KERNEL 0
#define USER_USER 1

#define ARDS_SIZE 32

static const u32 ARDS_TYPE_MEMORY = 1;
static const u32 ARDS_TYPE_RESERVED = 2;

typedef u32 *Page;

typedef struct ARDS
{
    u32 addr0;
    u32 addr1;
    u32 size0; // length
    u32 size1;
    u32 type;
} ARDS;

typedef struct PageEntry
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
} _packed PageEntry;

void init_memory();

extern u32 total_memory_bytes;
extern u32 ards_count;
extern ARDS ards_table[ARDS_SIZE];
extern u32 free_pages;

extern u32 page_alloc(u32 user, u32 size);
extern void page_free(u32 user, u32 page, u32 size);

u32 get_paddr(u32 vaddr);

#endif