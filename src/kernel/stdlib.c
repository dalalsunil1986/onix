#include <onix/stdlib.h>

u32 round_up(u32 number, u32 size)
{
    return (number + size - 1) / size;
}