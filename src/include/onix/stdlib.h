/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-22
*/

#ifndef ONIX_STDLIB_H
#define ONIX_STDLIB_H

#include <onix/types.h>

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

u32 round_up(u32 number, u32 size);

#endif