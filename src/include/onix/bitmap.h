/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-21
*/

#include <onix/types.h>

typedef struct
{
    u32 length;
    u8 *bits;
} bitmap_t;

void bitmap_init(bitmap_t *bitmap, char *buffer, u32 length);
bool bitmap_test(bitmap_t *bitmap, u32 index);
void bitmap_set(bitmap_t *bitmap, u32 index, bool value);
int bitmap_scan(bitmap_t *bitmap, u32 count);
