#ifndef ONIX_FILE_H
#define ONIX_FILE_H

#include <onix/types.h>
#include <fs/onix/fs.h>

#define MAX_OPEN_FILES 1024
#define FILE_NULL -1

typedef int32 fd_t;

typedef struct File
{
    u32 offset;
    u32 flag;
    Inode *inode;
} File;

typedef enum FileFlag
{
    O_R,    // 只读
    O_W,    // 只写
    O_RW,   // 读写
    O_C = 4 // 创建
} FileFlag;

typedef enum std_fd
{
    stdin = 0,
    stdout = 1,
    stderr = 2
} std_fd;

#ifdef ONIX_KERNEL_DEBUG
#define fs_function _
#define file_open _file_open
#endif

#endif