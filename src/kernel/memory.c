#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/assert.h>
#include <onix/string.h>

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

extern Page get_pde();

static u32 pde_idx(u32 idx)
{
    return idx >> 10;
}

static u32 pte_idx(u32 address)
{
    return address & 0b1111111111;
}

int scan_page(Bitmap *map, u32 size)
{
    int start = bitmap_scan(map, size);
    assert(start != -1);
    for (size_t i = start; i < start + size; i++)
    {
        bitmap_set(map, i, 1);
        DEBUGK("page alloc address 0x%04X \n", i);
    }
    return start;
}

u32 scan_physical_page(u32 size)
{
    u32 page = scan_page(&pmap, size);
    return page + base_physical_pages;
}

u32 get_pte(u32 idx)
{
    Page pde = get_pde();
    u32 pidx = pde_idx(idx);
    DEBUGK("get pde index 0x%x pidx 0x%X\n", pde, pidx);
    PageEntry *entry = &pde[pidx];
    if (!entry->present)
    {
        DEBUGK("create pte entry 0x%x 0x%X presnet %d\n", idx, pidx, entry->present);
        // // BMB;
        assert(pmap.length);
        // scan_page(&mmap, 1);

        u32 page = scan_physical_page(1);

        DEBUGK("get new page entry 0x%x\n", page);

        free_pages--;

        u32 store = 0;
        PageEntry *mentry = &store;
        mentry->present = 1;
        mentry->write = 1;
        // mentry->dirty = 0;
        mentry->index = page;
        DEBUGK("set entry 0x%x \n", store);
        pde[pidx] = mentry;
    }
    return entry->index;
}

void set_page(u32 vidx, u32 pidx)
{
    u32 pte = get_pte(vidx);
    u32 store = 0;
    PageEntry *mentry = &store;
    DEBUGK("set page pte 0x%05X vaddr 0x%05X paddr 0x%05X idx 0x%05X \n", pte, vidx, pidx, pte_idx(pidx));
    mentry->present = 1;
    mentry->write = 1;
    mentry->index = pidx;

    Page page = pte << 12;
    page[pte_idx(pidx)] = store;
}

u32 get_page(u32 vidx)
{
    DEBUGK("get pte idx 0x%X\n", vidx);
    Page pte = get_pte(vidx) << 12;
    DEBUGK("get pte idx 0x%X\n", pte);
    u32 page = pte[pte_idx(vidx)];
    return page;
}

void mmap_free(u32 user, Bitmap *mmap, u32 idx)
{
    DEBUGK("mmap set 0x%X index 0x%X\n", mmap->bits, idx);
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

    DEBUGK("pmap length %d\n", pmap.length);

    u32 pages = available_pages / PG_SIZE + 1;
    u32 remain = available_pages % PG_SIZE;
    if (!remain) // 恰好整除的情况
        pages--;

    u32 pstart = scan_physical_page(pages);
    u32 vstart = scan_page(&mmap, pages) + KERNEL_BASE_PAGE_VIDX;

    // BMB;

    DEBUGK("pmap pages %d \n", pages);
    DEBUGK("vaddr 0x%08X paddr 0x%08X pages %d \n", vstart, pstart, pages);

    for (size_t i = 0; i < pages; i++)
    {
        u32 pidx = (pstart + i);
        u32 vidx = (vstart + i);
        DEBUGK("physical map pages %d, vidx 0x%X pidx 0x%X\n", i, vidx, pidx);
        set_page(vidx, pidx);
    }

    Bitmap temp;
    u32 vaddress = vstart * PG_SIZE;
    temp.bits = vaddress;
    temp.length = available_pages / 8;
    bitmap_init(&temp);

    memcpy(vaddress, pmap.bits, PG_SIZE);
    mmap.length = (0x7000 - 0x3000);

    pmap.bits = temp.bits;
    pmap.length = temp.length;

    DEBUGK("Available start page idx 0x%X\n", base_physical_pages);
    DEBUGK("Available kernel bits 0x%X\n", mmap.length);
    DEBUGK("Available memory pages 0x%X\n", available_pages);
    // BMB;
}

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
        DEBUGK("ARDS 0x%08X Addr 0x%08X Size 0x%08X Type %d \n",
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

u32 page_alloc(u32 user, u32 size)
{
    u32 pstart = scan_physical_page(size);
    u32 vstart = 0;
    if (user == USER_KERNEL)
    {
        vstart = (scan_page(&mmap, size) + KERNEL_BASE_PAGE_VIDX);
        for (size_t i = 0; i < size; i++)
        {
            // BMB;
            u32 pidx = pstart + i;
            u32 vidx = vstart + i;
            set_page(vidx, pidx);
        }
    }
    free_pages -= size;
    DEBUGK("Available memory 0x%08X \n", free_pages);
    return vstart;
}

void page_free(u32 user, u32 vidx, u32 size)
{
    u32 vstart = vidx;
    if (user == USER_KERNEL)
    {
        DEBUGK("free page vstart %X \n", vstart);
        for (size_t i = 0; i < size; i++)
        {
            u32 vidx = (vstart + i) - KERNEL_BASE_PAGE_VIDX;
            u32 pidx = (get_page(vidx) >> 12);
            DEBUGK("page free vidx 0x%X pidx 0x%X i 0x%X \n", vidx, pidx, i);
            mmap_free(user, &mmap, vidx);
            mmap_free(user, &pmap, pidx);
            free_pages += 1;
        }
    }
}

void init_memory()
{
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    DEBUGK("Total Memory Ards count %d\n", ards_count);
    ards = get_valid_ards();
    if (ards->addr0 < BASE_ADDR_LIMIT)
    {
        DEBUGK("Memory size too small that can't run kernel, sorry about it!!!\n");
        halt();
    }
    init_memory_map();

    // u32 gap = 0x1;
    // while (free_pages > gap)
    // {
    //     u32 vidx = page_alloc(USER_KERNEL, gap);
    //     DEBUGK("Get vidx 0x%X\n", vidx);
    //     // BMB;
    //     page_free(USER_KERNEL, vidx, gap);
    //     // BMB;
    // }
}
