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
#include <onix/syscall.h>

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

void update_tss_esp0(Task *task)
{
    tss.esp0 = (u32 *)((u32)task + PG_SIZE);
}

void task_pde_activate(Task *task)
{
    assert(task->pde != NULL);
    // DEBUGP("task activate 0x%08X pde 0x%08X\n", task, task->pde);
    set_cr3(get_paddr(task->pde));
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

void process_execute(Tasktarget *target, const char *name)
{
    DEBUGP("process execute 0x%08X name %s\n", target, name);
    // BMB;
    Task *task = page_alloc(1);
    Task *cur = running_task();
    DEBUGP("process execute 0x%08X alloc 0x%08X\n", target, task);
    task_init(task, name, DEFAULT_PRIORITY, USER_USER);
    create_user_mmap(task);
    task_create(task, process_start, target);
    create_user_pde(task);
    push_task(task);
    push_ready_task(task);
    task->ppid = cur->pid;
    task->pid = task->tid;
}

void process_wrapper(Tasktarget *target, void *args)
{
    sys_exit(target(args));
}

void process_start(Tasktarget target, void *args)
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
    frame->eip = process_wrapper;
    frame->eflags = (EFLAGS_IOPL0 | EFLAGS_MBS | EFLAGS_IF);

    frame->esp -= sizeof(ProcessArgs);
    ProcessArgs *pargs = (ProcessArgs *)frame->esp;
    pargs->eip = process_wrapper;
    pargs->target = target;
    pargs->args = args;

    interrupt_exit(frame);
}

extern void test_processa();

void test_process(void *args)
{
    process_execute(test_processa, "test process");
}

void init_process()
{
    CHECK_STACK;
    init_tss();
}