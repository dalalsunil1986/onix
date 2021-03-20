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

ARDS ards_table[ARDS_SIZE];
static ARDS *ards = NULL;

Page pde;

Bitmap pmap;
Bitmap kmap;

u32 physical_page_alloc(u32 size);

static u32 get_pde_index(PageEntry *entry)
{
    return entry->index >> 10;
}

static u32 get_pte_index(PageEntry *entry)
{
    return entry->index & 0b1111111111;
}

u32 get_pte_page(u32 vaddress)
{
    // BMB;
    PageEntry *entry = &vaddress;
    u32 pde_idx = get_pde_index(entry);
    DEBUGK("get pde index entry 0x%x 0x%X\n", entry->index, pde_idx);
    entry = &pde[pde_idx];
    if (!entry->present || entry->dirty)
    {
        DEBUGK("get new pte entry 0x%x 0x%X\n", entry->index, pde_idx);
        // BMB;
        assert(pmap.length);
        u32 page = physical_page_alloc(1);

        u32 store = 0;
        PageEntry *mentry = &store;
        PageEntry *pentry = &page;
        mentry->present = 1;
        mentry->write = 1;
        mentry->index = pentry->index;
        pde[pde_idx] = mentry;
    }
    u32 page = entry->index << 12;
    return page;
}

u32 set_page(Page pte, u32 vaddress, u32 paddress)
{
    PageEntry *ventry = &vaddress;
    u32 store = 0;
    PageEntry *mentry = &store;
    PageEntry *pentry = &paddress;
    DEBUGK("set page 0x%X vindex 0x%X pindex 0x%X idx 0x%X \n",
           pte, ventry->index, pentry->index, get_pte_index(ventry));

    mentry->present = 1;
    mentry->write = 1;
    mentry->index = pentry->index;
    pte[get_pte_index(ventry)] = store;
    return 0;
}

u32 get_page(Page pte, u32 vaddress)
{
    PageEntry *ventry = &vaddress;
    u32 page = pte[get_pte_index(ventry)];
    return page;
}

void mmap_set(u32 user, Bitmap *mmap, u32 address, u32 value)
{
    address &= ~KERNEL_ADDR_MASK;
    PageEntry *entry = (PageEntry *)&address;
    u32 index = entry->index;
    if (user == USER_KERNEL)
    {
        index -= PG_BASE;
    }
    DEBUGK("mmap set 0x%X page address 0x%X index %d\n", mmap->bits, address, index);
    bitmap_set(mmap, index, value);
}

static void init_memory_map()
{
    available_memory_bytes = ards->size0;
    available_pages = available_memory_bytes / 0x1000; // 4k
    free_pages = available_pages;
    pmap.length = available_pages / 8;

    DEBUGK("pmap length %d\n", pmap.length);

    PageEntry *entry = &ards->addr0;
    Page pte = get_pte_page(entry);
    assert(pte);

    u32 paddress = ards->addr0;
    u32 vaddress = paddress | KERNEL_ADDR_MASK;

    DEBUGK("Set base pmap vaddr 0x%X paddr 0x%X \n", vaddress, paddress);
    set_page(pte, vaddress, paddress);

    pmap.bits = ards->addr0 | KERNEL_ADDR_MASK;

    u32 pages = pmap.length / PG_SIZE + 1;
    u32 remain = pmap.length % PG_SIZE;
    if (!remain) // 恰好整除的情况
        pages--;

    DEBUGK("pmap pages %d \n", pages);
    for (size_t i = 1; i < pages; i++)
    {
        u32 address = (ards->addr0 + PG_SIZE * i);
        DEBUGK("additional pmap pages %d, address 0x%X\n", i, address);
        set_page(pte, address | KERNEL_ADDR_MASK, address);
    }

    kmap.bits = (u8 *)KERNEL_MAP_ADDR;
    kmap.length = PG_SIZE;
    bitmap_init(&kmap);
    bitmap_init(&pmap);
    for (size_t i = 0; i < pages; i++)
    {
        u32 address = (ards->addr0 + PG_SIZE * i);
        mmap_set(USER_KERNEL, &pmap, address, 1);
        mmap_set(USER_KERNEL, &kmap, address, 1);
    }
    DEBUGK("Available memory pages %d\n", available_pages);
    // BMB;
}

void init_memory()
{
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    DEBUGK("Total Memory Ards count %d\n", ards_count);

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
    pde = (u32 *)PG_DIR_ADDRESS;
    if (ards->addr0 < BASE_ADDR_LIMIT)
    {
        DEBUGK("Memory size too small that can't run kernel, sorry about it!!!\n");
        halt();
    }
    init_memory_map();
}

u32 scan_page(Bitmap *map, u32 size)
{
    u32 start = bitmap_scan(map, size);
    for (size_t i = start; i < start + size; i++)
    {
        bitmap_set(map, i, 1);
        DEBUGK("page alloc address %d \n", i);
    }
    return start * PG_SIZE;
}

u32 physical_page_alloc(u32 size)
{
    u32 start = scan_page(&pmap, size);
    u32 page = (PG_BASE * PG_SIZE) + start;
    DEBUGK("alloc physical page 0x%X size %d \n", start, size);
    return page;
}

// void physical_page_free(Page page, u32 size)
// {
//     for (size_t i = 0; i < size; i++)
//     {
//         mmap_set(USER_KERNEL, &pmap, page + i * PG_SIZE, 0);
//     }
// }

u32 page_alloc(u32 user, u32 size)
{
    u32 pstart = physical_page_alloc(size);
    DEBUGK("pstart page 0x%X\n", pstart);
    u32 vstart = 0;
    if (user == USER_KERNEL)
    {
        vstart = (scan_page(&kmap, size) + BASE_ADDR_LIMIT) | KERNEL_ADDR_MASK;
        DEBUGK("vstart page 0x%X\n", vstart);
        for (size_t i = 0; i < size; i++)
        {
            // BMB;
            u32 vaddress = vstart + i * PG_SIZE;
            DEBUGK("staart 0x%X vaddress 0x%X i 0x%X \n", vstart, vaddress, i);
            u32 pte = get_pte_page(vaddress);
            DEBUGK("PTE %X \n", pte);
            set_page(pte, vaddress, pstart + (i * PG_SIZE));
            DEBUGK("set page finish %X\n", vaddress);
        }
    }
    return vstart;
}

void page_free(u32 user, u32 page, u32 size)
{
    u32 vstart = page;
    if (user == USER_KERNEL)
    {
        DEBUGK("free page vstart %X \n", vstart);
        for (size_t i = 0; i < size; i++)
        {
            u32 vaddress = vstart + i * PG_SIZE;
            DEBUGK("start page free 0x%X vaddress 0x%X i 0x%X \n", vstart, vaddress, i);
            u32 pte = get_pte_page(vaddress);
            DEBUGK("PTE %X \n", pte);
            u32 paddress = get_page(pte, vaddress);
            mmap_set(user, &kmap, vaddress, 0);
            mmap_set(user, &pmap, paddress, 0);
            DEBUGK("set page finish %X\n", vaddress);
        }
    }
}
