#ifndef ONIX_FSDIR_H
#define ONIX_FSDIR_H 1

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

Dir *onix_open_root_dir(Partition *part);

bool onix_search_dir_entry(Partition *part, Dir *dir, char *name, DirEntry *entry);
void onix_create_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry);
bool onix_sync_dir_entry(Partition *part, Dir *parent, DirEntry *entry);

Dir *onix_dir_open(Partition *part, u32 nr);
void onix_dir_close(Partition *part, Dir *dir);

void init_dir();

#endif