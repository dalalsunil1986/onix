/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/memory.h>
#include <onix/debug.h>
#include <onix/string.h>
#include <onix/stdlib.h>
#include <onix/assert.h>

#define DIDX(vaddr) (vaddr >> 22)
#define TIDX(vaddr) (vaddr >> 12 & 0b1111111111)

u32 memory_base;
u32 memory_size;
u32 total_pages;
u32 free_pages;

#define base_page (memory_base / PAGE_SIZE)
#define used_pages (total_pages - free_pages)

static addr_t physical_addr;

void init_ards(ards_t *ards, u32 count)
{
    memory_base = 0;
    memory_size = 0;
    for (size_t i = 0; i < count; i++, ards++)
    {
        if (ards->type != 1)
            continue;
        if (ards->size > memory_size)
        {
            memory_size = ards->size;
            memory_base = ards->base;
        }
    }

    u32 offset = memory_base & 0xfff;

    if (offset != 0)
    {
        memory_base = (memory_base / PAGE_SIZE + 1) * PAGE_SIZE;
        memory_size -= offset;
    }

    total_pages = memory_size / PAGE_SIZE;
    free_pages = total_pages;

    INFOK("System Memory Base 0x%08X Size 0x%08X\n", memory_base, memory_size);
    INFOK("System Memory Total 0x%X\n", total_pages);
}

static void init_addr(addr_t *addr, u32 start, u8 *bits, u32 length)
{
    addr->start = start;
    bitmap_init(&addr->mmap, bits, length);
}

static void clean_page(void *page)
{
    memset(page, 0, PAGE_SIZE);
}

static void init_entry(page_entry_t *entry, u32 index)
{
    u32 *value = entry;
    *value = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

static u32 scan_page(addr_t *addr, u32 size)
{
    assert(size > 0);
    u32 index = bitmap_scan(&addr->mmap, size);
    if (index == EOF)
    {
        panic("Scan page fail!!!\n");
    }

    u32 page = addr->start + index * PAGE_SIZE;
    free_pages -= size;

    DEBUGK("scan page 0x%08X size %d\n", page, size);
    return page;
}

static u32 reset_page(addr_t *addr, u32 page, u32 size)
{
    assert((page % PAGE_SIZE) == 0);
    assert(size > 0);
    u32 index = (page - addr->start) >> 12;
    for (size_t i = 0; i < size; i++)
    {
        bitmap_set(&addr->mmap, index + i, 0);
    }
}

static page_table_t get_pde()
{
    return (page_table_t)(0xfffff000);
}

static page_table_t get_pte(u32 vaddr)
{
    page_table_t pde = get_pde();
    u32 didx = DIDX(vaddr);
    page_entry_t *entry = &(*pde)[didx];

    if (!entry->present)
    {
        assert(physical_addr.mmap.length);
        u32 page = scan_page(&physical_addr, 1);
        init_entry(entry, page >> 12);
    }

    page_table_t table = PDE_MASK | (didx << 12);
    return table;
}

static void set_page(u32 vaddr, u32 paddr)
{
    page_table_t pte = get_pte(vaddr);
    u32 idx = TIDX(vaddr);
    page_entry_t *entry = &(*pte)[idx];
    init_entry(entry, paddr >> 12);
}

static void init_kernel_mmap()
{
    DEBUGK("free pages %d\n", free_pages);

    page_table_t pde = get_pde();
    page_entry_t *entry;

    u32 index = 0;

    // 填充内核 页表，从 0x301 到 0x3fe 共 254 个位置
    for (size_t i = KERNEL_BASE_PTE; i < ENTRY_SIZE; i++)
    {
        entry = &(*pde)[i];
        if (!entry->present) // 这个只有 0x300 和 0x3ff 是已经存在的
        {
            init_entry(entry, base_page + index);
            index++;
            u32 addr = PDE_MASK | (i << 12);
            clean_page(addr);
        }
    }

    // 映射物理内存位图

    // 计算需要物理内存位图页数
    u32 pmap_pages = round_up(total_pages, PAGE_SIZE * 8);

    // 物理内存位图长度
    u32 length = pmap_pages * PAGE_SIZE;

    // 将物理内存位图放在 KERNEL_BASE_PAGE 开始的地方，连续存放 pmap_pages 个页面
    for (size_t i = 0; i < pmap_pages; i++)
    {
        set_page(KERNEL_BASE_PAGE + i * PAGE_SIZE, (base_page + index) << 12);
        index++;
    }

    // 初始化物理内存地址
    init_addr(&physical_addr, memory_base, KERNEL_BASE_PAGE, length);

    // 将已用的内存位置置为 1，目前物理内存位图为空，直接 scan
    bitmap_scan(&physical_addr.mmap, index);

    // 可用页数减去已用的数量
    free_pages -= index;

    DEBUGK("free pages %d used pages %d\n", free_pages, used_pages);
}

void init_memory()
{
    INFOK("Initializing Memory Management System...\n");
    init_kernel_mmap();
    u32 page;
    for (size_t i = 1; i < 10; i++)
    {
        page = scan_page(&physical_addr, i);
        reset_page(&physical_addr, page, i);
    }
}

page_table_t get_cr3()
{
    asm volatile("movl %cr3, %eax\n");
}

void set_cr3(page_table_t pde)
{
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}
