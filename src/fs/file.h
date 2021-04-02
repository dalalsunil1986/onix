#ifndef ONIX_FILE_H
#define ONIX_FILE_H

#include <onix/types.h>
#include <fs/onix/fs.h>

#define MAX_OPEN_FILES 1024

typedef int32 fd_t;

typedef struct File
{
    u32 offset;
    u32 flag;
    Inode *inode;
} File;

typedef enum std_fd
{
    stdin = 0,
    stdout = 1,
    stderr = 2
} std_fd;

#endif