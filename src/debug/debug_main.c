#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/string.h>
#include <fs/onix/fs.h>
#include <fs/path.h>
#include <fs/onix/inode.h>
#include <fs/onix/fs.h>
#include <fs/onix/fsbitmap.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern Partition *root_part;

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
    init_harddisk();
    init_fs();
    Partition *part = root_part;
    u32 idx = inode_bitmap_alloc(part);
    DEBUGP("Alloc inode bit %d\n", idx);
    bitmap_sync(part, idx, INODE_BITMAP);

    u32 lba = block_bitmap_alloc(part);
    DEBUGP("Alloc block bit %d\n", lba);
    bitmap_sync(part, idx, BLOCK_BITMAP);
}

int main()
{
    test_inode();
}