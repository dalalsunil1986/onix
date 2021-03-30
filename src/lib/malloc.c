#include <onix/malloc.h>
#include <onix/syscall.h>

void *malloc(size_t size)
{
    return sys_malloc(size);
}

void free(void *ptr)
{
    return sys_free(ptr);
}