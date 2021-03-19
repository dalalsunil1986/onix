#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>

u32 __clock_counter;

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
    __clock_counter++;

    char * current = (char *)V_MEM_BASE + (VGA_WIDTH - 1) * 2;
    *current = __clock_counter % 10 + 0x30;
    // printk("Clock interrupt counter %i\n", __clock_counter);
}
