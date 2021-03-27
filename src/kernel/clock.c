#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>
#include <onix/kernel/debug.h>

// #define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

u32 __global_ticks;

void init_pit()
{
    printk("Initializing PIT...\n");
    outb(PIT_CONTROL_PORT, RATE_GENERATOR);
    outb(TIMER0_PORT, (u8)TIMER_VALUE);
    outb(TIMER0_PORT, (u8)(TIMER_VALUE >> 8));
}

void clock_handler(int vector)
{
    assert(vector == 0x20);

    Task *cur = running_task();
    DEBUGP("Current task is 0x%X ticks 0x%X\n", cur, cur->ticks);
    assert(cur->magic == TASK_MAGIC);

    cur->ticks--;
    if (cur->ticks == 0)
    {
        cur->ticks = cur->priority;
        schedule();
    }

    __global_ticks++;
    char ch = ' ';
    if ((__global_ticks % 2) != 0)
    {
        ch = 'C';
    }

    show_char(ch, 79, 0);
}

void init_clock()
{
    register_handler(IRQ_CLOCK, clock_handler);
    enable_irq(IRQ_CLOCK);
}