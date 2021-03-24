#include <onix/stdlib.h>

u32 round_up(u32 number, u32 size)
{
    u32 count = number / size;
    u32 remain = number % size;
    if (remain != 0)
        count += 1;
    return count * size;
}