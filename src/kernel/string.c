#include <onix/string.h>

size_t strlen(const char *str)
{
    register size_t len = 0;
    while (str[len] != 0)
    {
        len++;
    }
    return len;
}

void memcpy(void *dest, void *src, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        *((char *)(dest + i)) = *((char *)(src + i));
    }
}