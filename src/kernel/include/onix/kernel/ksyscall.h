#ifndef ONIX_KSYSCALL_H
#define ONIX_KSYSCALL_H

#define SYSCALL_SIZE 64

#include <onix/types.h>
#include <fs/file.h>

typedef void *SyscallHandler;

void init_syscall();

int32 __sys_chdir(const char *path);

int32 __sys_stat(const char *pathname, Stat *stat);

#endif