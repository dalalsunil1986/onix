#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>

u32 __clock_counter;

void init_pit()
{
    printk("Initializing PIT...\n");
    outb(PIT_CONTROL_PORT, RATE_GENERATOR);
    outb(TIMER0_PORT, (u8)TIMER_VALUE);
    outb(TIMER0_PORT, (u8)(TIMER_VALUE >> 8));
}