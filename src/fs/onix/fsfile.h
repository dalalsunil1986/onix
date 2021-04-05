#ifndef ONIX_FSFILE_H
#define ONIX_FSFILE_H

#include <onix/types.h>
#include <onix/kernel/harddisk.h>
#include <fs/onix/fsdir.h>
#include <fs/file.h>

bool onix_file_create(Partition *part, Dir *parent, OnixFile *file, char *name, FileFlag flags);
bool onix_file_open(Partition *part, OnixFile *file, u32 nr, FileFlag flags);
bool onix_file_close(Partition *part, OnixFile *file);
int32 onix_file_write(Partition *part, OnixFile *file, const void *content, int32 count);
int32 onix_file_read(Partition *part, OnixFile *file, const void *content, int32 count);
int32 onix_file_lseek(OnixFile *file, int32 offset, Whence whence);

#endif