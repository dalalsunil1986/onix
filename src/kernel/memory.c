#include <onix/kernel/memory.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/io.h>
#include <onix/kernel/debug.h>

u32 total_memory_bytes;
u32 ards_count;
u32 available_memory_bytes;
u32 available_pages;
u32 free_pages;

ARDS ards_table[ARDS_SIZE];
static ARDS *ards = NULL;

Page pde;

Bitmap mmap;

static u32 get_pde_index(PageEntry *entry)
{
    return entry->index >> 10;
}

static u32 get_pte_index(PageEntry *entry)
{
    return entry->index & 0b1111111111;
}

static Page get_pte_page(PageEntry *entry)
{
    u32 pde_idx = get_pde_index(entry);
    entry = &pde[pde_idx];
    if (!entry->present || entry->dirty)
        return NULL;
    Page page = entry->index << 12;
    return page;
}

static Page get_page(Page pte, PageEntry *entry)
{
}

static void set_page(Page pte, u32 paddress)
{
    PageEntry *entry = &paddress;
    u32 store = 0;
    PageEntry *mentry = &store;
    mentry->present = 1;
    mentry->write = 1;
    mentry->index = entry->index;
    pte[get_pte_index(entry)] = store;
}

static void mmap_set(u32 paddress, u32 value)
{
    PageEntry *entry = (PageEntry *)&paddress;
    u32 index = entry->index - PG_BASE;
    printk("page index %d\n", index);
    bitmap_set(&mmap, index, value);
}

static void init_memory_map()
{
    available_memory_bytes = ards->size0;
    available_pages = available_memory_bytes / 0x1000; // 4k
    free_pages = available_pages;
    mmap.length = available_pages / 8;

    PageEntry *entry = &ards->addr0;
    Page pte = get_pte_page(entry);
    set_page(pte, ards->addr0);
    mmap.bits = ards->addr0 | KERNEL_ADDR_MASK;
    mmap_set(ards->addr0, 1);

    u32 pages = mmap.length / PG_SIZE;
    u32 remain = mmap.length % PG_SIZE;
    if (!remain) // 恰好整除的情况
        pages--;
    for (size_t i = 1; i <= pages; i++)
    {
        u32 address = (ards->addr0 + PG_SIZE * i);
        printk("additional mmap pages %d, address 0x%X\n", i, address);
        set_page(pte, address);
        mmap_set(address, 1);
    }
    printk("Available memory pages %d\n", mmap.length);
}

void init_memory()
{
    printk("Initializing Memory...\n");
    printk("Total Memory Size 0x%X B\n", total_memory_bytes);
    printk("Total Memory Ards count %d\n", ards_count);

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
        printk("ARDS 0x%08X Addr 0x%08X Size 0x%08X Type %d \n",
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
        printk("Memory size too small that can't run kernel, sorry about it!!!\n");
        halt();
    }
    init_memory_map();
}

Page palloc(u32 size)
{
    u32 start = bitmap_scan(&mmap, size);
    printk("get start page %d size %d\n", start, size);
    for (size_t i = start; i < start + size; i++)
    {
        bitmap_set(&mmap, i, 1);
        printk("page alloc address %d \n", i);
    }
    return start;
}
