#include <onix/string.h>
#include <onix/queue.h>
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

void __init_kernel()
{
    // BMB;
    init_gdt();
    init_pid();
    make_setup_task();
    init_memory();
    init_arena();
    init_interrupt();
    init_task();
    init_process();
    init_harddisk();
    init_fs();
}
