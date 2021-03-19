#ifndef ONIX_TIMER_H
#define ONIX_TIMER_H

#include <onix/types.h>

static const IRQ0_FREQUENCY = 100;
static const INPUT_FREQUENCY = 1193182L;
static const RATE_GENERATOR = 0b00110100;
static const TIMER_VALUE = INPUT_FREQUENCY / IRQ0_FREQUENCY;
static const TIMER0_PORT = 0x40;
static const PIT_CONTROL_PORT = 0x43;

extern u32 __clock_counter;

void init_pit();

void clock_handler(int vector);

#endif