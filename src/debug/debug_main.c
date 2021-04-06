#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/pid.h>
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
#include <fs/onix/fssyscall.h>
#include <onix/kernel/ksyscall.h>
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

void test_read_write()
{
    char *buf = malloc(BLOCK_SIZE);
    partition_read(part, 1, buf, 2);
    partition_write(part, 1, buf, 2);
    free(buf);
}

void test_fsbitmap()
{
    u32 idx = onix_inode_bitmap_alloc(part);
    DEBUGP("Alloc inode bit %d\n", idx);
    onix_bitmap_sync(part, idx, INODE_BITMAP);

    onix_inode_bitmap_rollback_sync(part, idx);

    idx = onix_block_bitmap_alloc(part);
    DEBUGP("Alloc block bit %d\n", idx);
    onix_bitmap_sync(part, idx, BLOCK_BITMAP);
    onix_block_bitmap_rollback_sync(part, idx);
}

void test_inode()
{
    // PBMB;
    u32 idx = 5;
    u32 nr = 5;
    DEBUGP("inode open %d\n", nr);
    Inode *inode = onix_inode_open(part, nr);
    for (size_t i = 0; i < DIRECT_BLOCK_CNT; i++)
    {
        idx = onix_block_bitmap_alloc_sync(part);
        inode->blocks[i] = idx;
        inode->size += BLOCK_SIZE;
    }

    DEBUGP("inode sync %d\n", nr);
    onix_inode_sync(part, inode);
    DEBUGP("inode delete %d\n", nr);
    onix_inode_delete(part, nr);
    DEBUGP("inode close %d\n", nr);
    onix_inode_close(part, inode);
    DEBUGP("inode erase %d\n", nr);
    onix_inode_erase(part, nr);
}

void test_dir()
{
    DEBUGP("size of entry %d\n", sizeof(DirEntry));
    // PBMB;

    DEBUGP("test dir open root dir\n");

    Dir *root_dir = onix_open_root_dir(part);
    u32 nr;

    DirEntry entry;
    char filename[] = "hello";

    DEBUGP("test dir search file %s\n", filename);
    bool exists = onix_search_dir_entry(part, root_dir, filename, &entry);
    if (!exists)
    {
        // PBMB;
        DEBUGP("test dir alloc inode bitmap\n");
        nr = onix_inode_bitmap_alloc_sync(part);
        DEBUGP("file %s is not exists, then create it.\n", filename);
        onix_create_dir_entry(filename, nr, FILETYPE_REGULAR, &entry);
        onix_sync_dir_entry(part, root_dir, &entry);
    }
    else
    {
        DEBUGP("file %s is exists, congratulations!!!\n");
    }
    // PBMB;
    SearchRecord *record = malloc(sizeof(SearchRecord));
    memset(record, 0, sizeof(SearchRecord));
    // PBMB;
    nr = onix_search_file(filename, record);
    DEBUGP("search file %s nr %d\n", filename, nr);
    // PBMB;

    DEBUGP("delete dir entry ....\n");
    onix_delete_dir_entry(part, root_dir, &entry);
    free(record);
}

void test_file()
{
    // PBMB;
    OnixFile holder;
    OnixFile *file = &holder;
    file->flags = 0;
    file->inode = NULL;
    file->part = NULL;
    file->offset = 0;

    char filename[] = "A file.txt";
    Dir *root = onix_open_root_dir(part);
    bool success = onix_file_create(part, root, file, filename, O_C);
    u32 nr = file->inode->nr;

    DEBUGP("create file success is %d\n", success);
    DEBUGP("create file nr %u\n", file->inode->nr);

    DEBUGP("close file\n");
    onix_file_close(file);

    // PBMB;
    char buf[] = "hello world this is a file content\0";

    onix_file_open(part, file, nr, O_R | O_W);

    // PBMB;

    onix_file_write(file, buf, sizeof(buf));

    // PBMB;
    memset(buf, 0, sizeof(buf));

    file->offset = 0;

    onix_file_read(file, buf, sizeof(buf));
    DEBUGP("%s\n", buf);
    // PBMB;

    onix_file_close(file);

    onix_sys_unlink(filename);
}

void test_fssys_call()
{
    // PBMB;
    char filename[] = "/testfile";
    fd_t fd = onix_sys_open(filename, O_C | O_RW);
    DEBUGP("sys open file %d\n", fd);
    // PBMB;
    char str[] = "hello world this is a file content\0";
    onix_sys_write(fd, str, sizeof(str));
    // PBMB;
    char *buf = malloc(sizeof(str));
    memset(buf, 0, sizeof(str));

    // onix_sys_close(fd);
    // fd = onix_sys_open(filename, O_C | O_RW);

    onix_sys_lseek(fd, 0, SEEK_SET);

    onix_sys_read(fd, buf, sizeof(str));
    DEBUGP("read file %s content:\n    %s\n", filename, buf);
    onix_sys_close(fd);
    free(buf);
    onix_sys_unlink(filename);

    char d[] = "/testdir";

    onix_sys_mkdir(d);
    Dir *dir = onix_sys_opendir(d);
    // DEBUGP("dir %s size %d \n", d, dir->inode->size);

    onix_sys_closedir(dir);
    Dir *root = onix_sys_opendir("/");

    u32 count = 2;
    while (count--)
    {
        while (true)
        {
            DirEntry *entry = onix_sys_readdir(root);
            if (entry == NULL)
                break;
            DEBUGP("%s\n", entry->filename);
        }
        onix_sys_rewinddir(root);
    }
    onix_sys_rmdir(d);
    while (true)
    {
        DirEntry *entry = onix_sys_readdir(root);
        if (entry == NULL)
            break;
        DEBUGP("%s\n", entry->filename);
    }
}

extern char *__sys_getcwd(char *buf, u32 size);

test_cwd()
{
    char buf[MAX_PATH_LEN];
    memset(buf, 0, MAX_PATH_LEN);

    __sys_getcwd(buf, MAX_PATH_LEN);

    DEBUGP("get task cwd %s\n", buf);
}

#ifdef ONIX_KERNEL_DEBUG
int main()
{
    free_pages = 1000;
    Task *task = running_task();
    init_pid();
    make_setup_task();
    init_harddisk();
    init_fs();
#else
void test_function()
{
#endif
    part = root_part;
    print_format_info(part, part->super_block);
    test_read_write();
    // test_fsbitmap();
    // test_inode();
    // test_dir();
    // test_file();
    test_fssys_call();
    test_cwd();
    DEBUGP("Debug finish.....\n");
#ifdef ONIX_KERNEL_DEBUG
    task_destory(task);
#endif
}
