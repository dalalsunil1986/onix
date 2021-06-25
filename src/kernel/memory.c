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

// 物理地址位图
static addr_t physical_addr;

// 内核虚拟地址位图
static addr_t kernel_addr;

void init_ards(ards_t *ards, u32 count)
{
    INFOK("Machine ARDS count %d\n", count);
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
    if (addr == &physical_addr)
    {
        free_pages -= size;
    }

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

    if (addr == &physical_addr)
    {
        free_pages += size;
    }
}

static page_table_t get_pde()
{
    return (page_table_t)(0xfffff000);
}

static page_table_t get_pte(u32 vaddr, bool create)
{
    page_table_t pde = get_pde();
    u32 didx = DIDX(vaddr);
    page_entry_t *entry = &(*pde)[didx];

    if (!create)
    {
        assert(entry->present);
    }

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
    page_table_t pte = get_pte(vaddr, true);
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
    u32 ppages = round_up(total_pages, PAGE_SIZE * 8);

    // 物理内存位图长度
    u32 length = ppages * PAGE_SIZE;

    // 将物理内存位图放在 KERNEL_BASE_PAGE 开始的地方，连续存放 ppages 个页面
    for (size_t i = 0; i < ppages; i++)
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

    // 内核虚拟内存位图起始地址
    u32 vstart = KERNEL_BASE_PAGE + length;

    // 内核虚拟内存页数
    u32 vpages = KERNEL_ADDR_SIZE / PAGE_SIZE;

    // 内核虚拟内存位图需要的页数
    u32 vmap_pages = round_up(vpages, PAGE_SIZE * 8);

    for (size_t i = 0; i < vmap_pages; i++)
    {
        u32 paddr = scan_page(&physical_addr, 1);
        set_page(vstart + i * PAGE_SIZE, paddr);
    }

    // 初始化内核虚拟内存地址
    init_addr(&kernel_addr, KERNEL_BASE_PAGE, vstart, vmap_pages * PAGE_SIZE);

    // 将已用的虚拟位置置为 1，目前物理内存位图为空，直接 scan
    bitmap_scan(&kernel_addr.mmap, ppages + vmap_pages);

    DEBUGK("free pages %d used pages %d\n", free_pages, used_pages);
}

page_table_t get_cr3()
{
    asm volatile("movl %cr3, %eax\n");
}

void set_cr3(page_table_t pde)
{
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

u32 get_paddr(u32 vaddr)
{
    page_table_t pte = get_pte(vaddr, false);

    u32 idx = TIDX(vaddr);
    page_entry_t *entry = &(*pte)[idx];
    assert(entry->present);

    u32 paddr = ((u32)entry->index << 12) | vaddr & 0xfff;
    return paddr;
}

u32 kalloc_page(u32 size)
{
    assert(size < free_pages);
    u32 paddr = scan_page(&physical_addr, size);
    u32 vaddr = scan_page(&kernel_addr, size);
    for (size_t i = 0; i < size; i++)
    {
        set_page(vaddr + i * PAGE_SIZE, paddr + i * PAGE_SIZE);
    }
    DEBUGK("free pages %d used pages %d\n", free_pages, used_pages);
    return vaddr;
}

void kfree_page(u32 vaddr, u32 size)
{
    assert(size > 0);
    u32 paddr = get_paddr(vaddr);
    reset_page(&physical_addr, paddr, size);
    reset_page(&kernel_addr, vaddr, size);
    DEBUGK("free pages %d used pages %d\n", free_pages, used_pages);
}

void init_memory()
{
    INFOK("Initializing Memory Management System...\n");
    init_kernel_mmap();
}