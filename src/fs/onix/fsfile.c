#include <fs/onix/fsfile.h>
#include <fs/onix/fsbitmap.h>
#include <fs/onix/inode.h>
#include <fs/onix/fsdir.h>
#include <fs/file.h>
#include <onix/malloc.h>
#include <onix/string.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

bool file_create(Partition *part, Dir *parent, File *file, char *name, u8 flag)
{
    void *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        printk("Onix memory allocate failed!!!\n");
        return false;
    }

    u32 nr = inode_bitmap_alloc(part);
    if (nr == -1)
    {
        printk("Onix inode allocate failed!!!\n");
        return false;
    }

    bool success = false;
    u8 step = 0;
    Inode *inode = malloc(sizeof(Inode));
    if (inode == NULL)
    {
        step = 1;
        goto rollback;
    }
    inode_init(nr, inode);
    file->inode = inode;
    file->offset = 0;
    file->inode->write_deny = false;
    file->flag = flag;

    DirEntry entry;
    memset(&entry, 0, sizeof(DirEntry));

    create_dir_entry(name, nr, FILETYPE_REGULAR, &entry);

    if (!sync_dir_entry(parent, &entry, buf))
    {
        printk("Onix sync directory fail!!!\n");
        step = 2;
        goto rollback;
    }
    inode_sync(part, parent->inode);
    inode_sync(part, inode);
    bitmap_sync(part, nr, INODE_BITMAP);
    queue_push(&part->open_inodes, &inode->node);
    inode->open_cnts = 1;
    success = true;

rollback:
    switch (step)
    {
    case 2:
        free(inode);
    case 1:
        inode_bitmap_rollback(part, nr);
    case 0:
        free(buf);
    default:
        break;
    }
    return success;
}