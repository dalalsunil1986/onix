#include <onix/kernel/clock.h>
#include <onix/kernel/io.h>
#include <onix/kernel/printk.h>
#include <onix/kernel/assert.h>
#include <onix/kernel/interrupt.h>
#include <onix/kernel/task.h>

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
    char ch = (char)(__clock_counter % 10 + 0x30);
    show_char(ch, 79, 0);
    schedule();
}

void init_clock()
{
    handler_table[ICW2_INT_VECTOR_IRQ0 + IRQ_CLOCK] = clock_handler;
}