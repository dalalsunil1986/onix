/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-23
*/

#include <onix/thread.h>
#include <onix/memory.h>
#include <onix/string.h>
#include <onix/interrupt.h>
#include <onix/debug.h>

static void thread_wrapper(thread_target_t target, int argc, char const *argv)
{
    target(argc, argv);
    while (1)
    {
        /* code */
    }
}

thread_t *thread_init(
    thread_target_t target, int argc, char const *argv,
    char *name, int priority, u32 user)
{
    thread_t *thread = kalloc_page(1);
    memset(thread, 0, PAGE_SIZE);

    thread->magic = THREAD_MAGIC;
    thread->stack = (u32)thread + PAGE_SIZE;
    thread->pde = get_cr3();
    thread->user = user;
    thread->priority = priority;
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
};

thread_t *current_thread()
{
    asm volatile(
        "movl %esp, %eax\n"
        "and $0xfffff000, %eax\n");
}

void test_task()
{
    while (true)
    {
        DEBUGK("test task...\n");
    }
}

void init_thread()
{
    thread_t *current = current_thread();
    current->pde = get_cr3();
    current->magic = THREAD_MAGIC;

    thread_t *thread = thread_init(test_task, 0, NULL, "test", 1, 0);
    thread_switch(thread);
}