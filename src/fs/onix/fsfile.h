#ifndef ONIX_FSFILE_H
#define ONIX_FSFILE_H

#include <onix/types.h>
#include <onix/kernel/harddisk.h>
#include <fs/onix/fsdir.h>
#include <fs/file.h>

bool onix_file_create(Partition *part, Dir *parent, File *file, char *name, FileFlag flag);
bool onix_file_open(Partition *part, File *file, u32 nr, FileFlag flag);
bool onix_file_close(Partition *part, File *file);
int32 onix_file_write(Partition *part, File *file, const void *content, int32 count);

#endif