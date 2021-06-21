/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-21
*/

#include <onix/types.h>

typedef struct Bitmap
{
    u32 length;
    u8 *bits;
} Bitmap;

void bitmap_init(Bitmap *bitmap, char *buffer, u32 length);
bool bitmap_test(Bitmap *bitmap, u32 index);
void bitmap_set(Bitmap *bitmap, u32 index, bool value);
int bitmap_scan(Bitmap *bitmap, u32 count);
