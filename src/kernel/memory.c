#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/assert.h>
#include <onix/kernel/mutex.h>
#include <onix/string.h>
#include <onix/stdlib.h>

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

Lock memory_lock;

Bitmap pmap;
Bitmap mmap;

#define DIDX(vaddr) (vaddr >> 22)
#define TIDX(vaddr) (vaddr >> 12 & 0b1111111111)
#define SWAP_ADDR1 0xff400000
#define SWAP_ADDR2 0xff000000

extern u32 get_cr3();

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

static u32 scan_bitmap_page(Bitmap *map, u32 size)
{
    assert(size > 0 && size < map->length * 8);
    int start = bitmap_scan(map, size);
    if (start == -1)
    {
        panic("Scan page fail!!!\n");
    }
    for (size_t i = start; i < start + size; i++)
    {
        bitmap_set(map, i, 1);
        DEBUGP("page alloc address 0x%04X \n", i);
    }
    return start * PG_SIZE;
}

static u32 scan_physical_page(u32 size)
{
    u32 page = scan_bitmap_page(&pmap, size);
    free_pages -= size;
    return page + (base_physical_pages * PG_SIZE);
}

static PageTable get_pde()
{
    return (u32 *)(0xfffff000);
}

static PageTable get_pte(u32 vaddr)
{
    PageTable pde = get_pde();
    u32 didx = DIDX(vaddr);
    // DEBUGP("get pde index 0x%x pidx 0x%X\n", pde, didx);
    PageEntry *entry = &pde[didx];
    if (!entry->present)
    {
        // DEBUGP("create pte entry 0x%x 0x%X presnet %d\n", pde, didx, entry->present);
        // BMB;
        assert(pmap.length);

        u32 page = scan_physical_page(1);

        // DEBUGP("get new page entry 0x%x\n", page);

        u32 store = 0;
        PageEntry *mentry = &store;
        mentry->present = 1;
        mentry->write = 1;
        mentry->user = 1;
        mentry->index = page >> 12;

        // DEBUGP("set didx 0x%x entry 0x%x \n", didx, store);
        pde[didx] = store;
    }
    PageTable table = (PageTable *)(KERNEL_PDE_MASK | (DIDX(vaddr) << 12));
    // DEBUGP("get pte 0x%08X\n", table);
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
    mentry->user = 1;
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

void mmap_free(Bitmap *mmap, u32 idx)
{
    DEBUGP("mmap set 0x%X idx 0x%X\n", mmap->bits, idx);
    bitmap_set(mmap, idx, 0);
}

void page_copy(u32 dest, u32 src)
{
    int old = disable_int();
    set_page(SWAP_ADDR1, dest & 0xfffff000);

    set_page(SWAP_ADDR2, src & 0xfffff000);

    memcpy(SWAP_ADDR1, SWAP_ADDR2, PAGE_SIZE);

    PageTable pte;
    pte = get_pte(SWAP_ADDR1);
    pte[TIDX(SWAP_ADDR1)] = 0;

    pte = get_pte(SWAP_ADDR2);
    pte[TIDX(SWAP_ADDR2)] = 0;

    set_interrupt_status(old);
}

void create_user_mmap(struct Task *task)
{
    DEBUGP("Create user map 0x%X\n", task);
    task->vaddr.start = USER_VADDR_START;
    u32 length = (KERNEL_ADDR_MASK - USER_VADDR_START) / PG_SIZE / 8;
    u32 page_size = round_up(length, PG_SIZE);
    task->vaddr.mmap.bits = page_alloc(page_size);
    task->vaddr.mmap.length = length;
    bitmap_init(&task->vaddr.mmap);
}

void create_user_pde(struct Task *task)
{
    u32 page = page_alloc(1);
    u32 kernel_entry = 0x300 * 4; // 0xC00;

    PageTable *table = (PageTable)page;

    memcpy((void *)page, (void *)(0xFFFFF000), PG_SIZE); // todo replace with use pde
    // memcpy((void *)page + kernel_entry, (void *)(0xFFFFF000 + kernel_entry), 0x100 * 4);
    DEBUGP("get user pde page addr 0x%08X\n", page);

    u32 store = 0;
    PageEntry *entry = (PageEntry *)&store;

    entry->index = get_paddr(page) >> 12;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    table[1023] = store;
    task->pde = page;

    DEBUGP("set user pde self addr 0x%08X\n", store);
}

void free_user_pde(struct Task *task)
{
    int old_int = disable_int();
    u32 old_cr3 = get_cr3();

    set_cr3(get_paddr(task->pde));

    PageTable pde = get_pde();
    for (size_t i = 1; i < 0x300; i++) // 高位内存是内核的
    {
        // todo 暂时将低位 4 M 放过，留给内核
        PageEntry *pde_entry = &pde[i];
        if (!pde_entry->present)
            continue;

        PageTable pte = get_pte(i << 22);

        for (size_t j = 0; j < 1024; j++)
        {
            PageEntry *pte_entry = &pte[j];
            if (!pte_entry->present)
                continue;
            u32 vaddr = (i << 22 | j << 12);
            DEBUGP("memory exists 0x%08X\n", vaddr);
            free_task_page(vaddr, 1, task);
        }
    }
    DEBUGP("restore set cr3 \n");
    set_cr3(old_cr3);
    free_task_page(task->pde, 1, running_task());
    set_interrupt_status(old_int);
}

static void set_pages(u32 vstart, u32 pstart, u32 pages)
{
    for (size_t i = 0; i < pages; i++)
    {
        u32 paddr = (pstart + i * PG_SIZE);
        u32 vaddr = (vstart + i * PG_SIZE);
        DEBUGP("physical map pages %d, vidx 0x%X pidx 0x%X\n", i, vaddr, paddr);
        set_page(vaddr, paddr);
    }
}

static u32 scan_page(Task *task, u32 size)
{
    Bitmap *mmap = &task->vaddr.mmap;
    u32 start = task->vaddr.start;
    u32 vstart = scan_bitmap_page(mmap, size) + start;
    DEBUGP("Scan task page mmap 0x%08X bits 0x%08X start 0x%08X\n", mmap, mmap->bits, start);
    return vstart;
}

u32 scan_task_page(u32 size)
{
    Task *task = running_task();
    assert(task->magic == TASK_MAGIC);
    // assert(task->user == 0);
    u32 pstart = scan_physical_page(size);
    u32 vstart = scan_page(task, size);
    set_pages(vstart, pstart, size);
    return vstart;
}

void free_task_page(Page vaddr, u32 size, Task *task)
{
    Bitmap *mmap = &task->vaddr.mmap;
    u32 start = task->vaddr.start;

    // assert(task->user == 0);

    u32 vstart = (u32)vaddr;
    for (size_t i = 0; i < size; i++)
    {
        u32 vaddr = vstart + i * PG_SIZE;
        u32 vidx = (vaddr - start) >> 12;

        PageTable pte = get_pte(vaddr);
        PageEntry *entry = &pte[TIDX(vaddr)];
        assert(entry->present);

        entry->present = 0;
        u32 pidx = (pte[TIDX(vaddr)] >> 12) - base_physical_pages;
        DEBUGP("page free vidx 0x%X pidx 0x%X i 0x%X \n", vidx, pidx, i);
        mmap_free(mmap, vidx);
        mmap_free(&pmap, pidx);
    }
    free_pages += size;
}

static void init_memory_params()
{
    DEBUGP("init memory parameters\n");
    base_physical_pages = ards->addr0 / 0x1000;
    available_memory_bytes = ards->size0;
    available_pages = available_memory_bytes / 0x1000; // 4k
    free_pages = available_pages;

    DEBUGP("Available start page 0x%X\n", base_physical_pages);
    DEBUGP("Available memory pages 0x%X\n", available_pages);
}

static void init_kernel_mmap()
{
    DEBUGP("init kernel mmap\n");

    Task *task = running_task();
    task->pde = get_cr3();

    pmap.bits = (u8 *)PREPARE_PMAP_ADDR;
    pmap.length = PG_SIZE;
    bitmap_init(&pmap);

    Bitmap mmap;

    mmap.bits = (u8 *)KERNEL_MMAP_ADDR;
    mmap.length = PG_SIZE;
    bitmap_init(&mmap);

    u32 length = available_pages / 8;
    u32 pages = round_up(length, PG_SIZE) * 2; // 用于表示物理内存的位图的页数和内核虚拟地址的页数
    u32 pstart = scan_physical_page(pages);
    u32 vstart = scan_bitmap_page(&mmap, pages) + KERNEL_BASE_PAGE;

    DEBUGP("pmap pages %d \n", pages);
    DEBUGP("vaddr 0x%08X paddr 0x%08X pages %d \n", vstart, pstart, pages);

    set_pages(vstart, pstart, pages);

    memcpy(vstart, pmap.bits, PG_SIZE); // 将临时地址的内存拷贝到目标地址
    pmap.bits = (u8 *)vstart;
    pmap.length = length;
    DEBUGP("pstart 0x%08X \n", vstart);

    vstart += (pages / 2) * PG_SIZE;

    memcpy(vstart, mmap.bits, PG_SIZE);

    task->vaddr.start = KERNEL_BASE_PAGE;
    task->vaddr.mmap.bits = (u8 *)vstart;
    task->vaddr.mmap.length = length;
    DEBUGP("vstart 0x%08X \n", vstart);

    DEBUGP("init task:\n    start 0x%08X\n    bits 0x%08X\n    pmap  0x%08X\n    length %d",
           task->vaddr.start, task->vaddr.mmap.bits, pmap.bits, length);
}

static void init_memory_map()
{
    init_memory_params();
    init_kernel_mmap();
}

Page page_alloc(u32 size)
{
#ifdef ONIX_KERNEL_DEBUG
    return malloc(4 * 1024 * size);
#endif
    assert(size > 0 && size < available_pages);
    acquire(&memory_lock);
    u32 addr = scan_task_page(size);
    DEBUGP("Available memory 0x%08X \n", free_pages);
    release(&memory_lock);
    return addr;
}

void page_free(Page vaddr, u32 size)
{
#ifdef ONIX_KERNEL_DEBUG
    return free(vaddr);
#endif
    acquire(&memory_lock);
    free_task_page(vaddr, size, running_task());
    release(&memory_lock);
}

void test_memory()
{
    test_bitmap();
    DEBUGK("Test memory.....\n");
    char buf[0x10];
    u32 fp = free_pages;

    size_t size = 1;

    while (size <= 64)
    {
        // BMB;
        Page page = page_alloc(size);
        DEBUGP("Allocate page 0x%X size\n", page);
        u32 *value = (u32 *)page;
        *value = (u32)value;
        assert(*value == (u32)value);
        DEBUGP("free page 0x%X\n", page);
        page_free(page, size);
        size *= 2;
    }

    u32 pages[5];

    for (size_t i = 0; i < 5; i++)
    {
        u32 page = page_alloc(1);
        DEBUGP("Allocate page 0x%X\n", page);
        pages[i] = page;
    }
    for (size_t i = 0; i < 5; i++)
    {
        u32 page = pages[i];
        DEBUGP("free page 0x%X\n", page);
        page_free(page, 1);
    }
    assert(fp == free_pages);
    DEBUGK("Test memory finish.....\n");
}

void init_memory()
{
    CHECK_STACK;
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    DEBUGP("Total Memory Ards count %d\n", ards_count);
    DEBUGP("Init lock...\n");

    lock_init(&memory_lock);
    ards = get_valid_ards();
    if (ards->addr0 < BASE_ADDR_LIMIT)
    {
        panic("Memory size too small that can't run kernel, sorry about it!!!\n");
    }
    init_memory_map();
    test_memory();
}

u32 get_paddr(u32 vaddr)
{
    PageTable pte = get_pte(vaddr);

    // DEBUGP("get pte 0x%X\n", (u32)pte);
    u32 idx = TIDX(vaddr);
    PageEntry *page = (PageEntry *)&pte[idx];
    assert(page->present);

    u32 paddr = ((u32)page->index << 12) | vaddr & 0xfff;
    // DEBUGP("get vaddr 0x%X paddr 0x%X\n", vaddr, paddr);
    return paddr;
}
