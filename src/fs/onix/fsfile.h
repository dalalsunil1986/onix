#ifndef ONIX_FSFILE_H
#define ONIX_FSFILE_H

#include <onix/types.h>
#include <onix/kernel/harddisk.h>
#include <fs/onix/fsdir.h>
#include <fs/file.h>

bool file_create(Partition *part, Dir *parent, File *file, char *name, u8 flag);

#endif