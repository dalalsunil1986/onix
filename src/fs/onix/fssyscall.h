#ifndef ONIX_FSCALL_H
#define ONIX_FSCALL_H

#include <fs/file.h>

fd_t onix_sys_open(const char *pathname, FileFlag flags);
fd_t onix_sys_close(fd_t fd);
int32 onix_sys_write(fd_t fd, const void *buf, u32 count);
int32 onix_sys_read(fd_t fd, void *buf, u32 count);
int32 onix_sys_lseek(fd_t fd, int32 offset, Whence whence);
int32 onix_sys_unlink(const char *pathname);

#endif