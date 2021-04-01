#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/fs.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

int main()
{
    DEBUGP("hello, debug\n");
    init_harddisk();
    init_fs();
}