/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-20
*/

#ifndef ONIX_VSPRINTF_H
#define ONIX_VSPRINTF_H

#include <onix/stdarg.h>

extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char *buf, const char *fmt, ...);

#endif
