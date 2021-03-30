#ifndef ONIX_MALLOC_H
#define ONIX_MALLOC_H

#include <onix/types.h>

void *malloc(size_t size);
void free(void *ptr);

#endif