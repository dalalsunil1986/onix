/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#ifndef ONIX_CLOCK_H
#define ONIX_CLOCK_H

#include <onix/types.h>

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193182L
#define RATE_GENERATOR 0b00110100
#define TIMER_VALUE (INPUT_FREQUENCY / IRQ0_FREQUENCY)
#define TIMER0_REG 0x40
#define PIT_CTRL_REG 0x43
#define INTERRUPT_MILLISECONDS (1000 / IRQ0_FREQUENCY)

extern u32 global_ticks;

void init_pit();
void init_clock();

void clock_handler(int vector);

#endif