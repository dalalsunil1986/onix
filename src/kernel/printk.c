#include <onix/kernel/printk.h>

uint32 cpos = 0;

void put_char(char ch)
{
    char *current = (char *)VIDEO_BASE_ADDRESS + cpos * CHAR_SIZE;
    *current = ch;
    cpos += 1;
}