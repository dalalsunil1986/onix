#include <onix/string.h>
#include <onix/queue.h>
#include <onix/assert.h>
#include <onix/kernel/global.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/memory.h>
#include <onix/kernel/arena.h>
#include <onix/kernel/pid.h>
#include <onix/kernel/task.h>
#include <onix/kernel/process.h>
#include <onix/kernel/mutex.h>
#include <onix/kernel/harddisk.h>
#include <fs/onix/fs.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern void test_function();
extern Task *idle;

u32 __init_kernel()
{
    init_gdt();
    init_pid();
    init_setup_task();
    init_memory();
    init_arena();
    init_interrupt();
    init_tasks();
    init_process();
    init_harddisk();
    init_fs();
    init_file();

    // #ifndef ONIX_KERNEL_DEBUG
    //     test_function();
    //     PBMB;
    // #endif
    test_process();
    return idle->stack;
}
