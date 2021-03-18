#ifndef ONIX_TIMER_H
#define ONIX_TIMER_H

#include <onix/types.h>

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193182L
#define RATE_GENERATOR 0b00110100
#define TIMER_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define TIMER0_PORT 0x40
#define PIT_CONTROL_PORT 0x43

extern u32 __clock_counter;

void init_pit();

#endif