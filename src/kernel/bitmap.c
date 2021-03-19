#include <onix/bitmap.h>
#include <onix/string.h>

void bitmap_init(Bitmap *bitmap)
{
    memset(bitmap->bits, 0, bitmap->length);
}

int bitmap_test(Bitmap *bitmap, u32 idx)
{
    u32 idx_byte = idx / 8;
    u8 idx_bit = idx % 8;
    return ((u8)bitmap->bits[idx_byte] & (BITMAP_MASK << idx_bit));
}

int bitmap_scan(Bitmap *bitmap, u32 count)
{
    u32 idx_byte = 0;
    while ((bitmap->bits[idx_byte] == 0xff) && (idx_byte < bitmap->length))
    {
        idx_byte++;
    }
    if (idx_byte >= bitmap->length)
        return -1;
    u32 idx_bit = 0;
    while ((u8)(BITMAP_MASK << idx_bit) & bitmap->bits[idx_byte])
    {
        idx_bit++;
    }
    int start = idx_byte * 8 + idx_bit;
    u32 bit_left = bitmap->length * 8 - start;
    u32 next_bit = start + 1;
    u32 counter = 1;
    while (bit_left-- > 0)
    {
        if (counter == count)
        {
            start = next_bit - count + 1;
            break;
        }

        if (!(bitmap_scan(bitmap, next_bit)))
            counter++;
        else
            count = 0;

        next_bit++;
    }
    return start;
}

void bitmap_set(Bitmap *bitmap, u32 idx, u8 value)
{
    u32 idx_byte = idx / 8;
    u32 idx_bit = idx % 8;

    if (value)
    {
        bitmap->bits[idx_byte] |= (BITMAP_MASK << idx_bit);
    }
    else
    {
        bitmap->bits[idx_byte] &= ~(BITMAP_MASK << idx_bit);
    }
}