#include <onix/kernel/harddisk.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/pid.h>
#include <onix/kernel/printk.h>
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
#include <onix/assert.h>
#include <onix/malloc.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern Partition *root_part;
extern void print_format_info(Partition *part, SuperBlock *sb);
extern void osh_task();

static Partition *part;

void test_path()
{
    DEBUGP("hello, debug\n");

    char path[] = "../hello//world//onix/path\\.\\..\\but/here\\..\\there/is\\a wonderful land\\path.c\\\\//";
    char name[16] = {0};
    char *dir = path;
    while (dir)
    {
        DEBUGP("%s -> %s \n", dir, name);
        memset(name, 0, sizeof(name));
        dir = dirname(dir, name);
    }

    char buf[64];
    memset(buf, 0, sizeof(buf));

    abspath(path, buf);

    DEBUGP("path depth %d\n", path_depth(path));
    DEBUGP("path basename %s\n", basename(path, name));
    DEBUGP("path abspath %s\n", buf);
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
    u32 nr = onix_inode_bitmap_alloc_sync(part);
    DEBUGP("inode open %d\n", nr);
    Inode *inode = onix_inode_open(part, nr);
    for (size_t i = 0; i < 3; i++)
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
    DEBUGP("test dir open root dir\n");
    Dir *parent = onix_open_root_dir(part);

    DirEntry holder;
    DirEntry *entry = &holder;

    char filename[] = "file     ";
    u32 count = 1;
    while (count++ && (parent->inode->size / sizeof(DirEntry)) < 100)
    {
        sprintf(filename, "file %04d", count);
        DEBUGP("%s \n", filename);

        u32 nr = onix_inode_bitmap_alloc_sync(part);
        onix_init_dir_entry(filename, nr, FILETYPE_REGULAR, entry);
        bool success = onix_sync_dir_entry(part, parent, entry);
        if (!success)
            break;
        DEBUGP("Size %d\n", parent->inode->size / sizeof(DirEntry));
    }

    parent->offset = 0;
    int item = 0;
    while (item < 100)
    {
        DirEntry *child = onix_dir_read(part, parent);
        if (child == NULL)
            break;
        item++;
    }

    char filesearch[] = "file 0012";
    DEBUGP("search file %s\n", filesearch);
    bool exists = onix_search_dir_entry(part, parent, filesearch, entry);

    SearchRecord *record = malloc(sizeof(SearchRecord));
    memset(record, 0, sizeof(SearchRecord));

    int nr = onix_search_file(filesearch, record);
    DEBUGP("search file %s nr %d\n", filesearch, nr);

    if (nr != FILE_NULL)
    {
        DEBUGP("delete dir entry ....\n");
        onix_delete_dir_entry(part, parent, entry->filename);
    }

    free(record);
    DEBUGP("Size %d\n", parent->inode->size / sizeof(DirEntry));
}

void test_mkdir()
{
    onix_sys_mkdir("/hello");
    onix_list_dir("/");
    onix_sys_mkdir("/hello/test");
    onix_list_dir("/hello");
    onix_sys_rmdir("/hello/test");
    onix_list_dir("/hello");
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

    char filename[] = "file.txt";
    Dir *root = onix_open_root_dir(part);
    bool success = onix_file_create(part, root, file, filename, O_C);
    u32 nr = file->inode->nr;

    DEBUGP("create file success is %d\n", success);
    DEBUGP("create file nr %u\n", file->inode->nr);

    DEBUGP("close file\n");
    onix_file_close(file);

    // PBMB;
    char buf[] = "the quick brown fox jumps over a lazy dog and last bit of 64 bit";

    onix_file_open(part, file, nr, O_R | O_W);

    // PBMB;

    Inode *inode = file->inode;

    while (true)
    {
        int size = onix_file_write(file, buf, 64);
        if (size < 0)
            break;
        DEBUGP("write file size %d KB\n", file->inode->size / 1024);
    }

    // PBMB;
    memset(buf, 0, sizeof(buf));

    file->offset = 0;
    while (true)
    {
        int size = onix_file_read(file, buf, 64);
        if (size < 0)
            break;
        if (file->offset % 1024 == 0)
        {
            DEBUGP("%s offset %d\n", buf, file->offset);
        }
    }

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
    Stat stat;

    char *path = malloc(MAX_PATH_LEN);

    u32 count = 2;
    while (count--)
    {
        while (true)
        {
            DirEntry *entry = onix_sys_readdir(root);
            if (entry == NULL)
                break;
            memset(path, 0, MAX_PATH_LEN);
            path[0] = '/';
            memcpy(path + 1, entry->filename, strlen(entry->filename));
            onix_sys_stat(path, &stat);
            DEBUGP("file: %s nr: %d size: %d type: %d\n", path, stat.nr, stat.size, stat.type);
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
    free(path);
}

extern char *__sys_getcwd(char *buf, u32 size);
extern int32 __sys_chdir(const char *path);

test_cwd()
{
    char *buf = malloc(MAX_PATH_LEN);
    memset(buf, 0, MAX_PATH_LEN);

    __sys_getcwd(buf, MAX_PATH_LEN);
    DEBUGP("get task cwd %s\n", buf);
    __sys_chdir("/testdir");
    __sys_getcwd(buf, MAX_PATH_LEN);
    DEBUGP("get task cwd %s\n", buf);

    bool ex = exists("/");
    DEBUGP("root dir exits %d\n", ex);

    ex = exists("/testdir");
    DEBUGP("/testdir exits %d\n", ex);
    free(buf);
}

#ifdef ONIX_KERNEL_DEBUG
int main()
{
    free_pages = 1000;
    Task *task = running_task();
    init_pid();
    init_setup_task();
    init_harddisk();
    init_fs();
#else
void test_function()
{
#endif
    part = root_part;
    print_format_info(part, part->super_block);
    // test_path();
    // test_read_write();
    // test_fsbitmap();
    // test_inode();
    // test_dir();
    test_file();
    // test_fssys_call();
    // test_cwd();
    // test_mkdir();
    DEBUGP("Debug finish.....\n");
#ifdef ONIX_KERNEL_DEBUG
    task_destory(task);
#endif
    // osh_task();
}
