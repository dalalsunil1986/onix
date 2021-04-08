#include <onix/kernel/global.h>
#include <onix/kernel/process.h>
#include <onix/kernel/pid.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>
#include <onix/assert.h>
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
extern void interrupt_exit(InterruptFrame *frame);
extern void __interrupt_exit();
extern u32 create_user_mmap(Task *task);
extern void create_user_pde(struct Task *task);
extern void copy_user_pde(Task *parent, Task *task);

void init_tss()
{
    DEBUGK("init TSS...\n\0");
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

void process_pde_activate(Task *task)
{
    assert(task->pde != NULL);
    // DEBUGP("task activate 0x%08X pde 0x%08X\n", task, task->pde);
    set_cr3(get_paddr(task->pde));
}

void process_activate(Task *task)
{
    // DEBUGP("process activate 0x%08X user %d\n", task, task->user);
    assert(task != NULL);
    process_pde_activate(task);
    // BMB;
    if (task->user != 0)
    {
        update_tss_esp0(task);
    }
}

void process_create(Task *task, Tasktarget target, int argc, char const *argv[])
{
    DEBUGK("taskwrapper 0x%X\n    process_start 0x%X\n    target 0x%X\n", task_wrapper, process_start, target);
    u32 stack = task->stack;
    stack -= sizeof(InterruptFrame);

    stack -= sizeof(TaskArgs);

    TaskArgs *args = stack;
    // args->process_start = process_start;
    args->eip = process_start;
    args->target = target;
    args->argc = argc;
    args->argv = argv;

    stack -= sizeof(TaskFrame);
    task->stack = stack;

    TaskFrame *frame = (TaskFrame *)task->stack;
    frame->eip = process_start;
    frame->ebp = 0x11111111; // 这里的值不重要，用于调试定位栈顶信息
    frame->ebx = 0x22222222;
    frame->edi = 0x33333333;
    frame->esi = 0x44444444;
}

void process_execute(Tasktarget *target, int argc, char const *argv[], const char *name)
{
    DEBUGP("process execute 0x%08X name %s\n", target, name);
    // BMB;
    Task *task = page_alloc(1);
    Task *cur = running_task();
    DEBUGP("process execute 0x%08X alloc 0x%08X\n", target, task);

    task_init(task, name, DEFAULT_PRIORITY, USER_USER);
    create_user_mmap(task);

    process_create(task, target, argc, argv);

    task->cwd = malloc(MAX_PATH_LEN);

    memset(task->cwd, 0, MAX_PATH_LEN);
    memcpy(task->cwd, "/", 1);

    create_user_pde(task);

    push_task(task);
    push_ready_task(task);

    task->ppid = cur->pid;
    task->pid = task->tid;
}

static void process_copy_task(Task *parent, Task *task)
{
    memcpy(task, parent, PG_SIZE);
    task->tid = allocate_pid();
    task->pid = task->tid;
    task->ppid = parent->pid;

    task->all_node.next = NULL;
    task->all_node.prev = NULL;
    task->node.next = NULL;
    task->node.prev = NULL;
    task->ticks = 0;

    u32 page_count = create_user_mmap(task);
    memcpy(task->vaddr.mmap.bits, parent->vaddr.mmap.bits, page_count * PG_SIZE);
    task->vaddr.mmap.length = parent->vaddr.mmap.length;
    strcat(task->name, "_fork");
}

static void process_build_stack(Task *task)
{
    u32 addr = (u32)task + PG_SIZE;

    addr -= sizeof(InterruptFrame);
    InterruptFrame *iframe = (InterruptFrame *)addr;

    addr -= sizeof(TaskFrame);
    TaskFrame *tframe = addr;

    iframe->eax = 0;

    tframe->ebp = 0xaa55aa55;
    tframe->ebx = 0xaa55aa55;
    tframe->edi = 0xaa55aa55;
    tframe->esi = 0xaa55aa55;

    tframe->eip = __interrupt_exit;

    task->stack = tframe;

    DEBUGP("interrupt exit 0x%X\n", __interrupt_exit);
}

Task *process_copy(Task *parent)
{
    bool old = disable_int();
    Task *task = page_alloc(1);
    DEBUGP("copy new task 0x%X\n", task);
    process_copy_task(parent, task);
    copy_user_pde(parent, task);
    process_build_stack(task);

    task->cwd = malloc(MAX_PATH_LEN);
    memcpy(task->cwd, parent->cwd, strlen(parent->cwd));

    // todo update open files...

    set_interrupt_status(old);
    return task;
}

void process_start(Tasktarget target, int argc, char const *argv[])
{
    Task *cur = running_task();
    DEBUGP("process start 0x%X\n", cur);

    cur->stack = (char *)cur + PG_SIZE - sizeof(InterruptFrame);

    InterruptFrame *frame = (InterruptFrame *)cur->stack;

    frame->edi = 1;
    frame->esi = 2;
    frame->ebp = 3;
    frame->esp_dummy = 4;

    frame->ebx = 5;
    frame->edx = 6;
    frame->ecx = 7;
    frame->eax = 8;

    frame->gs = 0;
    frame->ds = SELECTOR_USER_DATA;
    frame->es = SELECTOR_USER_DATA;
    frame->fs = SELECTOR_USER_DATA;
    frame->ss = SELECTOR_USER_DATA;
    frame->cs = SELECTOR_USER_CODE;

    frame->eip = task_wrapper;
    frame->eflags = (EFLAGS_IOPL0 | EFLAGS_MBS | EFLAGS_IF);

    frame->esp = (void *)((u32)page_alloc(1) + PG_SIZE);

    frame->esp -= sizeof(TaskArgs);
    TaskArgs *args = (TaskArgs *)frame->esp;
    args->eip = task_wrapper;
    args->target = target;
    args->argc = argc;
    args->argv = argv;

    interrupt_exit(frame);
}

void init_process()
{
    CHECK_STACK;
    init_tss();
}