#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/string.h>
#include <fs/onix/fs.h>
#include <fs/path.h>
#include <fs/file.h>
#include <fs/onix/inode.h>
#include <fs/onix/fs.h>
#include <fs/onix/fsbitmap.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/fsdir.h>
#include <fs/onix/fsfile.h>
#include <onix/malloc.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern Partition *root_part;
extern void print_format_info(Partition *part, SuperBlock *sb);

static Partition *part;

void test_path()
{
    DEBUGP("hello, debug\n");

    char path[] = "/hello//world//onix/path\\\\\\there/is\\a wonderful land\\path.c\\\\//";
    char name[16] = {0};
    char *dir = path;
    while (dir)
    {
        DEBUGP("%s -> %s \n", dir, name);
        memset(name, 0, sizeof(name));
        dir = dirname(dir, name);
    }

    DEBUGP("path depth %d\n", path_depth(path));
    DEBUGP("path basename %s\n", basename(path, name));
}

void test_inode()
{
    u32 idx = 5;
    u32 lba = 0;
    idx = onix_inode_bitmap_alloc(part);
    DEBUGP("Alloc inode bit %d\n", idx);
    onix_bitmap_sync(part, idx, INODE_BITMAP);

    lba = onix_block_bitmap_alloc(part);
    DEBUGP("Alloc block bit %d\n", lba);
    onix_bitmap_sync(part, idx, BLOCK_BITMAP);

    for (size_t nr = 0; nr < 20; nr++)
    {
        Inode *inode = onix_inode_open(part, nr);
        for (size_t i = 0; i < DIRECT_BLOCK_CNT; i++)
        {
            idx = onix_inode_bitmap_alloc(part);
            onix_bitmap_sync(part, idx, INODE_BITMAP);
            inode->blocks[i] = idx;
            inode->size += BLOCK_SIZE;
        }
        onix_inode_sync(part, inode);
        onix_inode_close(part, inode);
    }
    lba = get_block_lba(part, idx);
    DEBUGP("data block lba 0x%08X idx %d\n", lba, idx);
}

void test_dir()
{
    DEBUGP("size of entry %d\n", sizeof(DirEntry));
    char buf[BLOCK_SIZE];
    Dir *root_dir = onix_open_root_dir(part);
    u32 nr = onix_inode_bitmap_alloc_sync(part);
    DirEntry entry;
    char filename[] = "hello";
    bool exists = onix_search_dir_entry(part, root_dir, filename, &entry);
    if (!exists)
    {
        DEBUGP("file %s is not exists, then create it.\n");
        onix_create_dir_entry(filename, nr, FILETYPE_REGULAR, &entry);
        onix_sync_dir_entry(part, root_dir, &entry);
    }
    else
    {
        DEBUGP("file %s is exists, congratulations!!!\n");
    }
}

void test_file()
{
    File holder;
    File *file = &holder;
    Dir *root = onix_open_root_dir(part);
    onix_file_create(part, root, file, "A file.txt", 1);

    char buf[] = "hello world this is a file content\0";
    DEBUGP("create file nr %d\n", file->inode->nr);

    onix_file_open(part, file, file->inode->nr, O_R | O_W);

    onix_file_write(part, file, buf, sizeof(buf));

    memset(buf, 0, sizeof(buf));

    file->offset = 0;

    onix_file_read(part, file, buf, sizeof(buf));

    onix_file_close(part, file);
}

int main()
{
    init_harddisk();
    init_fs();
    part = root_part;
    print_format_info(part, part->super_block);
    // test_inode();
    test_dir();
    // test_file();
}