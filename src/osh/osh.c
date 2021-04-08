#include "osh.h"
#include <onix/stdio.h>

#ifdef ONIX_KERNEL_DEBUG
int main(int argc, char const *argv[])
#else
int osh_task(int argc, char const *argv[])
#endif
{
    printf("hello world\n");
    return 0;
}
