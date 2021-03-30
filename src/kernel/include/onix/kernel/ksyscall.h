#ifndef ONIX_KSYSCALL_H
#define ONIX_KSYSCALL_H

#define SYSCALL_SIZE 16

typedef void *SyscallHandler;

void init_syscall();

#endif