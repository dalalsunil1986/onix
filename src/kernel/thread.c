/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#include <onix/thread.h>
#include <onix/memory.h>
#include <onix/string.h>
#include <onix/interrupt.h>
#include <onix/debug.h>
#include <onix/assert.h>

static list_t thread_list;
static thread_t *idle;

static void thread_wrapper(thread_target_t target, int argc, char const *argv)
{
    assert(!get_interrupt());
    set_interrupt(1);
    target(argc, argv);
    while (1)
    {
        /* code */
    }
}

static void push_thread(thread_t *thread)
{
    bool status = set_interrupt(false);
    assert(!list_find(&thread_list, &thread->node));
    list_push(&thread_list, &thread->node);
    set_interrupt(status);
}

static thread_t *get_thread(THREAD_STATUS status)
{
    bool intr = set_interrupt(false);
    thread_t *current = NULL;

    list_node_t *ptr = thread_list.tail.prev;

    for (; ptr != &thread_list.head; ptr = ptr->prev)
    {
        thread_t *thread = element_entry(thread_t, node, ptr);
        if (thread->status != status)
            continue;
        if (current == NULL || current->ticks < thread->ticks)
            current = thread;
    }
    set_interrupt(intr);
    // DEBUGK("get thread size %d\n", list_size(&thread_list));
    return current;
}

thread_t *thread_init(
    thread_target_t target, int argc, char const *argv,
    char *name, int priority, u32 user)
{
    thread_t *thread = kalloc_page(1);
    memset(thread, 0, PAGE_SIZE);

    thread->status = INIT;
    thread->magic = THREAD_MAGIC;
    thread->stack = (u32)thread + PAGE_SIZE;
    thread->pde = get_cr3();
    thread->user = user;
    thread->priority = priority;
    thread->ticks = priority;
    strcpy(thread->name, name);

    u32 stack = thread->stack;

    stack -= sizeof(thread_args_t);
    thread_args_t *args = stack;
    args->eip = thread_wrapper;
    args->target = target;
    args->argc = argc;
    args->argv = argv;

    stack -= sizeof(thread_frame_t);
    thread_frame_t *frame = stack;

    frame->ebx = 0x11111111;
    frame->ebp = 0x22222222;
    frame->esi = 0x33333333;
    frame->edi = 0x44444444;
    frame->eip = thread_wrapper;

    thread->stack = stack;

    thread->status = READY;
    push_thread(thread);
    DEBUGK("get thread size %d\n", list_size(&thread_list));
};

thread_t *current_thread()
{
    asm volatile(
        "movl %esp, %eax\n"
        "and $0xfffff000, %eax\n");
}

void schedule()
{
    assert(!get_interrupt());

    thread_t *next = get_thread(READY);
    assert(next->magic == THREAD_MAGIC);

    thread_t *current = current_thread();
    assert(current->magic == THREAD_MAGIC);

    if (current->status == RUNNING)
    {
        current->status = READY;
    }

    next->status = RUNNING;
    if (next == current)
        return;
    thread_switch(next);
}

extern void idle_task();
extern void init_task();

void init_thread()
{
    list_init(&thread_list);

    thread_t *current = current_thread();
    current->pde = get_cr3();
    current->magic = THREAD_MAGIC;
    current->priority = 1;
    current->ticks = 1;
    strcpy(current->name, "main");

    thread_t *idle = thread_init(idle_task, 0, NULL, "idle", 500, KERNEL_USER);
    thread_init(init_task, 0, NULL, "init", 1000, KERNEL_USER);
}