#include "osh.h"
#include <onix/stdio.h>

#ifdef MAIN_DEBUG
int main(int argc, char const *argv[])
#else
int osh_task(int argc, char const *argv[])
#endif
{
    clear();
    printf("hello world\n");
    return 0;
}
