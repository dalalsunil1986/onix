#include <onix/kernel/timer.h>
#include <onix/kernel/io.h>

u32 __clock_counter;

void set_frequency(u8 port, u8 number, u8 rwl, u8 mode, u16 value)
{
    outb(PIT_CONTROL_PORT, (u8)(number << 6 | rwl << 4 | mode << 1));
    outb(port, (u8)value);
    outb(port, (u8)(value >> 8));
}

void init_pit()
{
    set_frequency(
        COUNTER0_PORT,
        COUNTER0_NO,
        READ_WRITE_LATCH,
        COUNTER_MODE,
        COUNTER0_VALUE);
}