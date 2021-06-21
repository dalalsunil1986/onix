/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-21
*/

#include <onix/bitmap.h>
#include <onix/string.h>

void bitmap_init(Bitmap *bitmap, char *buffer, u32 length)
{
    memset(buffer, 0, length);
    bitmap->bits = buffer;
    bitmap->length = length;
}

bool bitmap_test(Bitmap *bitmap, u32 index)
{
    u32 bytes = index / 8;
    u8 bits = index % 8;
    return (bitmap->bits[bytes] & (1 << bits));
}

void bitmap_set(Bitmap *bitmap, u32 index, bool value)
{
    u32 bytes = index / 8;
    u32 bits = index % 8;

    if (value)
    {
        bitmap->bits[bytes] |= (1 << bits);
    }
    else
    {
        bitmap->bits[bytes] &= ~(1 << bits);
    }
}

int bitmap_scan(Bitmap *bitmap, u32 count)
{
    int start = EOF;
    u32 bits_left = bitmap->length * 8;
    u32 next_bit = 0;
    u32 counter = 0;

    while (bits_left-- > 0)
    {
        if (counter == count)
        {
            start = next_bit - count;
            break;
        }
        if (!bitmap_test(bitmap, next_bit))
        {
            counter++;
        }
        else
        {
            counter = 0;
        }
        next_bit++;
    }

    if (start == EOF)
        return EOF;

    bits_left = count;
    next_bit = start;
    while (bits_left--)
    {
        bitmap_set(bitmap, next_bit++, 1);
    }

    return start;
}