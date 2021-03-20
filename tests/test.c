#include <stdio.h>
#include <stdarg.h>
#include <onix/types.h>
#include <onix/bitmap.h>

#define SIZE 1024

int main()
{

    u8 bits[SIZE];
    Bitmap map;
    map.length = SIZE;
    map.bits = bits;

    bitmap_init(&map);

    u32 turns = 100;
    while (--turns)
    {
        u32 size = 5;
        u32 start = bitmap_scan(&map, size);

        for (size_t i = start; i < start + size; i++)
        {
            bitmap_set(&map, i, 1);
        }
    }

    return 0;
}