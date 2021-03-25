#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/assert.h>
#include <onix/string.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

u32 total_memory_bytes;
u32 ards_count;
u32 available_memory_bytes;
u32 available_pages;
u32 free_pages;
u32 base_physical_pages;

ARDS ards_table[ARDS_SIZE];
static ARDS *ards = NULL;

Bitmap pmap;
Bitmap mmap;

extern PageTable get_pde();

#define DIDX(vaddr) (vaddr >> 22)
#define TIDX(vaddr) (vaddr >> 12 & 0b1111111111)

static ARDS *get_valid_ards()
{
    u32 max_size = 0;
    u32 index;
    for (size_t i = 0; i < ards_count; i++)
    {
        ARDS var = ards_table[i];
        if (var.type != ARDS_TYPE_MEMORY)
            continue;
        if (var.size0 > max_size)
        {
            index = i;
            max_size = var.size0;
        }
    }
    if (max_size != 0)
    {
        ards = &ards_table[index];
        DEBUGP("ARDS 0x%08X Addr 0x%08X Size 0x%08X Type %d \n",
               ards, ards->addr0, ards->size0, ards->type);
        if (ards->addr0 % 0x1000 != 0)
        {
            ards->addr0 += 0x1000;
            ards->addr0 &= 0xfffff000;
            ards->size0 -= 0x1000;
        }
    }
    return ards;
}

static u32 scan_page(Bitmap *map, u32 size)
{
    int start = bitmap_scan(map, size);
    assert(start != -1);
    for (size_t i = start; i < start + size; i++)
    {
        bitmap_set(map, i, 1);
        DEBUGP("page alloc address 0x%04X \n", i);
    }
    return start * PG_SIZE;
}

static u32 scan_physical_page(u32 size)
{
    u32 page = scan_page(&pmap, size);
    free_pages -= size;
    return page + (base_physical_pages * PG_SIZE);
}

static PageTable get_pte(u32 vaddr)
{
    PageTable pde = get_pde();
    u32 didx = DIDX(vaddr);
    DEBUGP("get pde index 0x%x pidx 0x%X\n", pde, didx);
    PageEntry *entry = &pde[didx];
    if (!entry->present)
    {
        DEBUGP("create pte entry 0x%x 0x%X presnet %d\n", pde, didx, entry->present);
        // BMB;
        assert(pmap.length);

        u32 page = scan_physical_page(1);

        DEBUGP("get new page entry 0x%x\n", page);

        u32 store = 0;
        PageEntry *mentry = &store;
        mentry->present = 1;
        mentry->write = 1;
        mentry->user = 1;
        mentry->index = page >> 12;

        DEBUGP("set didx 0x%x entry 0x%x \n", didx, store);
        pde[didx] = store;
    }
    PageTable table = (PageTable *)(KERNEL_PDE_MASK | (DIDX(vaddr) << 12));
    DEBUGP("get pte 0x%08X\n", table);
    return table;
}

static void set_page(u32 vaddr, u32 paddr)
{
    PageTable pte = get_pte(vaddr);
    u32 store = 0;
    PageEntry *mentry = &store;
    DEBUGP("set page pte 0x%05X vaddr 0x%05X paddr 0x%05X idx 0x%05X \n", pte, vaddr, paddr, TIDX(paddr));
    mentry->present = 1;
    mentry->write = 1;
    mentry->index = paddr >> 12;
    u32 idx = TIDX(vaddr);
    DEBUGP("get map page 0x%08X store 0x%08X pidx 0x%X\n", pte, store, idx);
    pte[idx] = store;
}

u32 get_page(u32 vaddr)
{
    DEBUGP("get pte vaddr 0x%X\n", vaddr);
    PageTable pte = get_pte(vaddr);
    DEBUGP("get pte idx 0x%X\n", pte);
    u32 page = pte[TIDX(vaddr)];
    return page & 0xfffff000;
}

void mmap_free(u32 user, Bitmap *mmap, u32 idx)
{
    DEBUGP("mmap set 0x%X idx 0x%X\n", mmap->bits, idx);
    bitmap_set(mmap, idx, 0);
}

static void init_memory_map()
{
    base_physical_pages = ards->addr0 / 0x1000;
    available_memory_bytes = ards->size0;
    available_pages = available_memory_bytes / 0x1000; // 4k
    free_pages = available_pages;

    pmap.bits = (u8 *)PREPARE_PMAP_ADDR;
    pmap.length = PG_SIZE;
    bitmap_init(&pmap);

    mmap.bits = (u8 *)KERNEL_MMAP_ADDR;
    mmap.length = PG_SIZE;
    bitmap_init(&mmap);

    DEBUGP("pmap length %d\n", pmap.length);

    u32 pages = available_pages / PG_SIZE + 1;
    u32 remain = available_pages % PG_SIZE;
    if (!remain) // 恰好整除的情况
        pages--;

    u32 pstart = scan_physical_page(pages);
    u32 vstart = scan_page(&mmap, pages) + KERNEL_BASE_PAGE;

    DEBUGP("pmap pages %d \n", pages);
    DEBUGP("vaddr 0x%08X paddr 0x%08X pages %d \n", vstart, pstart, pages);

    for (size_t i = 0; i < pages; i++)
    {
        u32 paddr = (pstart + i * PG_SIZE);
        u32 vaddr = (vstart + i * PG_SIZE);
        DEBUGP("physical map pages %d, vidx 0x%X pidx 0x%X\n", i, vaddr, paddr);
        set_page(vaddr, paddr);
    }

    Bitmap temp;
    temp.bits = vstart;
    temp.length = available_pages / 8;
    bitmap_init(&temp);

    memcpy(vstart, pmap.bits, PG_SIZE);
    mmap.length = (0x7000 - 0x3000);

    pmap.bits = temp.bits;
    pmap.length = temp.length;

    DEBUGP("Available start page idx 0x%X\n", base_physical_pages);
    DEBUGP("Available kernel bits 0x%X\n", mmap.length);
    DEBUGP("Available memory pages 0x%X\n", available_pages);
}

Page page_alloc(u32 user, u32 size)
{
    assert(size > 0 && size < available_pages);

    u32 pstart = scan_physical_page(size);
    u32 vstart = 0;
    if (user == USER_KERNEL)
    {
        vstart = (scan_page(&mmap, size) + KERNEL_BASE_PAGE);
        for (size_t i = 0; i < size; i++)
        {
            u32 paddr = pstart + i * PG_SIZE;
            u32 vaddr = vstart + i * PG_SIZE;
            set_page(vaddr, paddr);
        }
    }
    DEBUGP("Available memory 0x%08X \n", free_pages);
    return vstart;
}

void page_free(u32 user, Page vaddr, u32 size)
{
    u32 vstart = (u32)vaddr;
    if (user == USER_KERNEL)
    {
        DEBUGP("free page vstart %X \n", vstart);
        for (size_t i = 0; i < size; i++)
        {
            u32 vaddress = vstart + i * PG_SIZE;
            u32 vidx = (vaddress >> 12) - KERNEL_BASE_PAGE_IDX;
            PageTable pte = get_pte(vaddress);
            PageEntry *entry = &pte[TIDX(vaddress)];
            assert(entry->present);

            entry->present = 0;
            u32 pidx = (pte[TIDX(vaddress)] >> 12) - base_physical_pages;
            DEBUGP("page free vidx 0x%X pidx 0x%X i 0x%X \n", vidx, pidx, i);
            mmap_free(user, &mmap, vidx);
            mmap_free(user, &pmap, pidx);
        }
        free_pages += size;
    }
}

void test_memory()
{
    DEBUGK("Test memory.....\n");
    char buf[0x10];
    u32 fp = free_pages;

    size_t size = 1;

    while (size <= 64)
    {
        Page page = page_alloc(USER_KERNEL, size);
        DEBUGP("Allocate page 0x%X size\n", page, size);
        u32 *value = (u32 *)page;
        *value = (u32)value;
        assert(*value == (u32)value);
        DEBUGP("free page 0x%X\n", page);
        page_free(USER_KERNEL, page, size);
        size *= 2;
    }
    assert(fp == free_pages);
}

void init_memory()
{
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    DEBUGP("Total Memory Ards count %d\n", ards_count);
    ards = get_valid_ards();
    if (ards->addr0 < BASE_ADDR_LIMIT)
    {
        DEBUGP("Memory size too small that can't run kernel, sorry about it!!!\n");
        halt();
    }
    init_memory_map();
    test_memory();
}

u32 get_paddr(u32 vaddr)
{
    PageTable pte = get_pte(vaddr);

    DEBUGP("get pte 0x%X\n", (u32)pte);
    u32 idx = TIDX(vaddr);
    PageEntry *page = (PageEntry *)&pte[idx];
    assert(page->present);

    u32 paddr = ((u32)page->index << 12) | vaddr & 0xfff;
    DEBUGP("get vaddr 0x%X paddr 0x%X\n", vaddr, paddr);
    return paddr;
}
