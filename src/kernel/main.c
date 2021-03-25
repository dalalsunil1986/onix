#include <onix/string.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/arena.h>
#include <onix/kernel/task.h>
#include <onix/kernel/process.h>
#include <onix/queue.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

void __init_kernel()
{
    // BMB;
    init_gdt();
    init_memory();

    init_interrupt();
    // init_task();
    // init_process();
    enable_int();
    init_arena();
}

int main()
{
    // test_queue();
    // clear();
    // printk("code selector 0x%X\n", *(short *)&SELECTOR_KERNEL_CODE);
    // printk("data selector 0x%X\n", *(short *)&SELECTOR_KERNEL_DATA);
    // printk("video selector 0x%X\n", *(short *)&SELECTOR_KERNEL_VIDEO);
    // printk("code descriptor segment %d\n", gdt[1].segment);

    // Queue queue;
    // u32 entry = element_entry(Queue, size, &queue.size);
    // DEBUGP("Queue entry 0x%X entry 0x%X \n", entry, &queue);

    u32 counter = 0;

    // char *queue = (char *)page_alloc(USER_KERNEL, 1);
    // queue_init(queue);
    // char buffer[32];

    // while (++counter)
    // {
    //     memset(buffer, 0, 32);
    //     show_char(counter % 10 + 0x30, 77, 0);
    //     DEBUGP("free pages 0x%X\n\0", free_pages);
    //     char *node = (char *)page_alloc(USER_KERNEL, 1);
    //     queue_push(queue, node);
    //     if (free_pages < 2)
    //         break;
    // }
    // DEBUGP("free pages 0x%X, start release memory...\n\0", free_pages);
    // while (!queue_empty(queue))
    // {
    //     DEBUGP("free pages 0x%X\n\0", free_pages);
    //     Node * node = queue_pop(queue);
    //     page_free(USER_KERNEL, node, 1);
    // }

    while (++counter)
    {
        char ch = (char)(counter % 10 + 0x30);
        show_char(ch, 77, 0);
        /* code */
    }

    return 0;
}