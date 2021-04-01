#include <stdio.h>
#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif


int main()
{
    DEBUGP("hello, debug\n");
}