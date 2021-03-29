#include <onix/kernel/global.h>
#include <onix/kernel/process.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/clock.h>
#include <onix/string.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

static TSS tss;

extern void set_cr3(u32 addr);
extern void create_user_mmap(Task *task);
extern void interrupt_exit(TaskFrame *frame);

void init_tss()
{
    printk("Initilaizing TSS...\n\0");
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = *(u32 *)&SELECTOR_KERNEL_DATA;
    tss.iobase = sizeof(tss);

    Descriptor *tss_desc = gdt + SELECTOR_KERNEL_TSS_INDEX;
    init_descriptor(tss_desc, get_paddr(&tss), sizeof(tss) - 1);
    tss_desc->granularity = 0;
    tss_desc->big = 0;
    tss_desc->long_mode = 0;
    tss_desc->present = 1;
    tss_desc->DPL = PL0;
    tss_desc->type = 0b1001;
    // tss_desc->code = 1;
    // tss_desc->conform_expand = 0;
    // tss_desc->read_write = 0; // busy
    // tss_desc->accessed = 1;
    load_tss(&SELECTOR_KERNEL_TSS);
}

void process_start(Tasktarget target)
{
    Task *cur = running_task();
    cur->stack += sizeof(ThreadFrame);
    TaskFrame *frame = (TaskFrame *)cur->stack;
    frame->edi = 0;
    frame->esi = 0;
    frame->ebp = 0;
    frame->esp_dummy = 0;

    frame->ebx = 0;
    frame->edx = 0;
    frame->ecx = 0;
    frame->eax = 0;

    frame->gs = 0;
    frame->ds = SELECTOR_USER_DATA;
    frame->es = SELECTOR_USER_DATA;
    frame->fs = SELECTOR_USER_DATA;
    frame->ss = SELECTOR_USER_DATA;
    frame->cs = SELECTOR_USER_CODE;
    frame->esp = (void *)((u32)page_alloc(1) + PG_SIZE);
    frame->eip = target;
    frame->eflags = (EFLAGS_IOPL0 | EFLAGS_MBS | EFLAGS_IF);

    // BMB;
    // 这个地方可能需要退栈
    interrupt_exit(frame);
}

void update_tss_esp0(Task *task)
{
    tss.esp0 = (u32 *)((u32)task + PG_SIZE);
}

void task_pde_activate(Task *task)
{
    assert(task->pde != NULL);
    // DEBUGP("task activate 0x%08X pde 0x%08X\n", task, task->pde);
    set_cr3(task->pde);
}

void process_activate(Task *task)
{
    // DEBUGP("process activate 0x%08X user %d\n", task, task->user);
    assert(task != NULL);
    task_pde_activate(task);
    // BMB;
    if (task->user != 0)
    {
        update_tss_esp0(task);
    }
}

void create_user_pde(Task *task)
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
    task->pde = get_paddr(page);
    DEBUGP("set user pde self addr 0x%08X\n", store);
}

void process_execute(Tasktarget *target, const char *name)
{
    DEBUGP("process execute 0x%08X name %s\n", target, name);
    // BMB;
    Task *task = page_alloc(1);
    DEBUGP("process execute 0x%08X alloc 0x%08X\n", target, task);
    task_init(task, name, DEFAULT_PRIORITY, USER_USER); // todo replace with user
    create_user_mmap(task);
    task_create(task, process_start, target);
    create_user_pde(task);
    push_task(task);
    push_ready_task(task);
}

void test_processa()
{
    u32 counter = 0;
    while (true)
    {
        counter++;
        char ch = ' ';
        if ((counter % 2) != 0)
        {
            ch = 'T';
        }
        show_char(ch, 73, 0);
        // DEBUGP("test process a .....\n");// 此时应该没有 io 权限无法修改 光标会报 gp 异常
        // sleep(100);
    }
}

void test_process()
{
    process_execute(test_processa, "test process");
}

void init_process()
{
    init_tss();
    // test_process();MAKE
}