#ifndef ONIX_STDIO_H
#define ONIX_STDIO_H

void clear();
int printf(const char *fmt, ...);

#define DEBUGF(fmt, args...) debugf(__BASE_FILE__, __LINE__, fmt, ##args)

#endif