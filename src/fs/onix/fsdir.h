#ifndef ONIX_FSDIR_H
#define ONIX_FSDIR_H 1

#include <fs/onix/fs.h>
#include <onix/kernel/harddisk.h>

Dir *onix_open_root_dir(Partition *part);

bool onix_search_dir_entry(Partition *part, Dir *parent, char *name, DirEntry *entry);
void onix_init_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry);
bool onix_sync_dir_entry(Partition *part, Dir *parent, DirEntry *entry);
bool onix_delete_dir_entry(Partition *part, Dir *parent, char *name);

int32 onix_search_file(const char *pathname, SearchRecord *record);
Dir *onix_dir_open(Partition *part, u32 nr);
DirEntry *onix_dir_read(Partition *part, Dir *dir);
void onix_dir_close(Partition *part, Dir *dir);
DirEntry *onix_dir_read(Partition *part, Dir *dir);
bool onix_dir_empty(Partition *part, Dir *dir);
bool onix_dir_remove(Partition *part, Dir *parent, Dir *dir, DirEntry *entry);

void init_dir();

#endif