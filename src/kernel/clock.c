#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/task.h>
#include <onix/stdlib.h>

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
    __global_ticks++;

    Task *cur = running_task();
    assert(cur->magic == TASK_MAGIC);

    cur->ticks--;
    if (cur->ticks == 0)
    {
        cur->ticks = cur->priority;
        schedule();
    }
}

void init_clock()
{
    register_handler(IRQ_CLOCK, clock_handler);
    enable_irq(IRQ_CLOCK);
}

static void ticks_to_sleep(u32 ticks)
{
    assert(ticks > 0);
    while (ticks--)
    {
        task_yield();
    }
}

void clock_sleep(u32 milliseconds)
{
    u32 sleep_ticks = round_up(milliseconds, INTERRUPT_MILLISECONDS);
    assert(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks);
}