#ifndef ONIX_KSYSCALL_H
#define ONIX_KSYSCALL_H

#define SYSCALL_SIZE 64

#include <onix/types.h>
#include <fs/file.h>

typedef void *SyscallHandler;

void init_syscall();

#endif