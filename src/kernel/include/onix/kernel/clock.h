#ifndef ONIX_TIMER_H
#define ONIX_TIMER_H

#include <onix/types.h>

static const int IRQ0_FREQUENCY = 100;
static const int INPUT_FREQUENCY = 1193182L;
static const int RATE_GENERATOR = 0b00110100;
static const int TIMER_VALUE = INPUT_FREQUENCY / IRQ0_FREQUENCY;
static const int TIMER0_PORT = 0x40;
static const int PIT_CONTROL_PORT = 0x43;
static const int INTERRUPT_MILLISECONDS = 1000 / IRQ0_FREQUENCY;

extern u32 __global_ticks;

void init_pit();
void init_clock();

void clock_handler(int vector);
void clock_sleep(u32 milliseconds);

#endif