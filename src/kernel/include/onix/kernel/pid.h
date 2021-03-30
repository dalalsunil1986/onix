#ifndef ONIX_PID_H
#define ONIX_PID_H

#define MAX_PID 65535

#include <onix/types.h>

typedef u32 pid_t;

void init_pid();
pid_t allocate_pid();
void release_pid(pid_t pid);
bool test_pid(pid_t pid);

#endif