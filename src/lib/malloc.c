#include <onix/malloc.h>
#include <onix/syscall.h>

#ifndef ONIX_KERNEL_DEBUG
void *malloc(size_t size)
{
    return sys_malloc(size);
}

void free(void *ptr)
{
    return sys_free(ptr);
}
#else
#include <malloc.h>
#endif