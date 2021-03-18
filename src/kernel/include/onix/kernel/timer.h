#ifndef ONIX_TIMER_H
#define ONIX_TIMER_H

#include <onix/types.h>

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

extern u32 __clock_counter;

void set_frequency(u8 port, u8 number, u8 rwl, u8 mode, u16 value);
void init_pit();

#endif