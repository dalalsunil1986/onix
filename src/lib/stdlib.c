/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#include <onix/stdlib.h>

u32 round_up(u32 number, u32 size)
{
    return (number + size - 1) / size;
}