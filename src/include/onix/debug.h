/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#ifndef ONIX_DEBUG_H
#define ONIX_DEBUG_H

#define BOCHS_MAGIC_BREAKPOINT asm("xchgw %bx, %bx")
#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif