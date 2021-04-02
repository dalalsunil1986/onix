#ifndef ONIX_FSDIR_H
#define ONIX_FSDIR_H 1

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

Dir *open_root_dir(Partition *part);

bool search_dir_entry(Partition *part, Dir *dir, char *name, DirEntry *entry);
void create_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry);
bool sync_dir_entry(Dir *parent, DirEntry *entry, void *buf);

Dir *dir_open(Partition *part, u32 nr);
void dir_close(Partition *part, Dir *dir);

void init_dir();

#endif