#ifndef ONIX_STDARG_H
#define ONIX_STDARG_H

#ifndef ONIX_KERNEL_DEBUG
typedef char *va_list;

#define va_start(ap, v) (ap = (va_list)&v + sizeof(char *))
#define va_arg(ap, t) (*(t *)((ap += sizeof(char *)) - sizeof(char *)))
#define va_end(ap) (ap = (va_list)0)

#else
#include <stdarg.h>
#endif

#endif