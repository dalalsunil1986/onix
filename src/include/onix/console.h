/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#ifndef ONIX_CONSOLE_H
#define ONIX_CONSOLE_H

#include <onix/types.h>

/* VGA */
#define CRT_ADDR_REG 0x3D4   /* CRT Controller Registers - Addr Register */
#define CRT_DATA_REG 0x3D5   /* CRT Controller Registers - Data Register */
#define START_ADDR_H 0xC     /* reg index of video mem start addr (MSB) */
#define START_ADDR_L 0xD     /* reg index of video mem start addr (LSB) */
#define CRT_CURSOR_HIGH 0xE  /* reg index of cursor position (MSB) */
#define CRT_CURSOR_LOW 0xF   /* reg index of cursor position (LSB) */
#define VGA_MEM_BASE 0xB8000 /* base of color video memory */
#define VGA_MEM_SIZE 0x8000  /* 32K: B8000H -> BFFFFH */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

u16 get_cursor();
void set_cursor(u16 cursor);

#endif