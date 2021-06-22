
#include <onix/clock.h>
#include <onix/interrupt.h>
#include <onix/debug.h>
#include <onix/assert.h>

u32 global_ticks;

void init_pit()
{
    printk("Initializing PIT...\n");
    outb(PIT_CTRL_REG, RATE_GENERATOR);
    outb(TIMER0_REG, (u8)TIMER_VALUE);
    outb(TIMER0_REG, (u8)(TIMER_VALUE >> 8));
}

void clock_handler(int vector)
{
    assert(vector == 0x20);
    global_ticks++;
    DEBUGK("clock handler %d\n", global_ticks);
}

void init_clock()
{
    global_ticks = 0;
    register_handler(IRQ_CLOCK, clock_handler);
    // set_request(IRQ_CLOCK, true);
}
