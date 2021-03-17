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