#include <fs/onix/fsdir.h>
#include <fs/onix/inode.h>

Dir root_dir;

void open_root_dir(Partition *part)
{
    root_dir.inode = inode_open(part, part->super_block->root_inode_nr);
    root_dir.dir_offset = 0;
}

void dir_close(Dir *dir)
{
    if (dir == &root_dir)
        return;
    // inode_close(dir->inode);
}

void create_dir_entry(char *filename, u32 nr, FileType type, DirEntry *entry)
{
}

bool sync_dir_entry(Dir *parent, DirEntry *dir, void *buf)
{
}