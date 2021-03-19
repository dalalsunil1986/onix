#ifndef ONIX_PRINTK_H
#define ONIX_PRINTK_H

#include <onix/types.h>
#include <onix/stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA */
#define CRTC_ADDR_REG 0x3D4 /* CRT Controller Registers - Addr Register */
#define CRTC_DATA_REG 0x3D5 /* CRT Controller Registers - Data Register */
#define START_ADDR_H 0xC    /* reg index of video mem start addr (MSB) */
#define START_ADDR_L 0xD    /* reg index of video mem start addr (LSB) */
#define CURSOR_H 0xE        /* reg index of cursor position (MSB) */
#define CURSOR_L 0xF        /* reg index of cursor position (LSB) */
#define V_MEM_BASE 0xB8000  /* base of color video memory */
#define V_MEM_SIZE 0x8000   /* 32K: B8000H -> BFFFFH */

extern int get_cursor();
extern void set_cursor(int pos);


void clear();
void put_char(char ch);

extern int vsprintf(char *buf, const char *fmt, va_list args);
int printk(const char *fmt, ...);

#endif