/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-18
*/

#include <onix/string.h>

namespace onix
{
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

    void memset(void *dest, char ch, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            *((char *)(dest + i)) = ch;
        }
    }

    char *strcpy(char *dest, char *src)
    {
        int i = 0;
        while (src[i])
        {
            dest[i] = src[i];
            i++;
        }
        dest[i] = 0;
        return dest;
    }

    char *strcat(char *dest, char *src)
    {
        char *ptr = dest;
        while (*ptr)
            ptr++;
        while (*src)
        {
            *(ptr++) = *(src++);
        }
        *ptr = 0;
        return dest;
    }

    int strcmp(const char *str1, const char *str2)
    {
        while (*str1 == *str2 && *str1 != NULL && *str2 != NULL)
        {
            str1++;
            str2++;
        }
        return *str1 < *str2 ? -1 : *str1 > *str2;
    }

    char *strchr(const char *str, const char ch)
    {
        while (*str != 0)
        {
            if (*str == ch)
            {
                return (char *)str;
            }
            str++;
        }
        return NULL;
    }

    char *strrchr(const char *str, const char ch)
    {
        const char *last_char = NULL;
        while (*str != 0)
        {
            if (*str == ch)
            {
                last_char = str;
            }
            str++;
        }
        return (char *)last_char;
    }

}