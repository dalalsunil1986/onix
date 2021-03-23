#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>

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
    assert(current_task->stack_magic == TASK_MAGIC);

    __global_ticks++;
    current_task->ticks--;
    if (current_task->ticks == 0)
    {
        current_task->ticks = current_task->priority;
        schedule();
    }

    char ch = (char)(__global_ticks % 10 + 0x30);
    show_char(ch, 79, 0);
}

void init_clock()
{
    register_handler(IRQ_CLOCK, clock_handler);
}