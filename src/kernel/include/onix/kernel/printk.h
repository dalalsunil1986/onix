#ifndef ONIX_PRINTK
#define ONIX_PRINTK

#include <onix/types.h>

#define VIDEO_BASE_ADDRESS 0xb8000
#define CHAR_SIZE 2;

extern uint32 cpos;

void put_char(char ch);

#endif