#ifndef ONIX_STRING_H
#define ONIX_STRING_H

#include <onix/types.h>

size_t strlen(const char *str);

void memcpy(void *dest, void *src, size_t size);
void memset(void *dest, char ch, size_t size);
char *strcpy(char *dest, char *src);
char *strcat(char *dest, char *src);
int strcmp(const char *str1, const char *str2);

char *strchr(const char *str, const char ch);
char *strrchr(const char *str, const char ch);

#endif