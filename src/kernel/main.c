/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#include <onix/console.h>
#include <onix/printk.h>
#include <onix/assert.h>
#include <onix/global.h>
#include <onix/bitmap.h>

int main()
{
    clear();
    init_gdt();

    char bits[1024];
    Bitmap map;
    bitmap_init(&map, bits, sizeof(bits));

    auto value = bitmap_test(&map, 0);
    bitmap_set(&map, 0, 1);
    value = bitmap_test(&map, 0);

    auto start = bitmap_scan(&map, 10);

    return 0;
}
