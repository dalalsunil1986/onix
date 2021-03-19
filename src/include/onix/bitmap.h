#ifndef ONIX_BITMAP_H
#define ONIX_BITMAP_H

#include <onix/types.h>

static const u8 BITMAP_MASK = 1;

typedef struct Bitmap
{
    u32 length;
    u8 *bits;
} Bitmap;

void bitmap_init(Bitmap *bitmap);
int bitmap_test(Bitmap *bitmap, u32 idx);
int bitmap_scan(Bitmap *bitmap, u32 count);
void bitmap_set(Bitmap *bitmap, u32 idx, u8 value);

#endif