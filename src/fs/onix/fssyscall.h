#ifndef ONIX_FSCALL_H
#define ONIX_FSCALL_H

#include <fs/file.h>

fd_t onix_sys_open(const char *pathname, FileFlag flags);
fd_t onix_sys_close(fd_t fd);
#endif