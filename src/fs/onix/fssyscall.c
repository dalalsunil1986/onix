#include <fs/onix/fssyscall.h>
#include <fs/onix/fsdir.h>
#include <fs/onix/fsfile.h>
#include <fs/onix/fsbitmap.h>
#include <fs/onix/fsblock.h>
#include <fs/onix/inode.h>
#include <fs/path.h>
#include <onix/string.h>
#include <onix/malloc.h>
#include <onix/kernel/task.h>
#include <onix/assert.h>
#include <onix/kernel/debug.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

extern Partition *root_part;
extern Dir root_dir;

static fd_t create_file(Partition *part, Dir *parent, char *filename, FileFlag flags)
{
    fd_t fd = get_free_global_fd();
    if (fd == FILE_NULL)
        return fd;
    File *file = get_global_file(fd);

    bool success = onix_file_create(part, parent, file, filename, flags);
    if (!success)
    {
        printk("create file failure!!!\n");
        return FILE_NULL;
    }
    Task *task = running_task();
    fd_t task_fd = task_install_fd(fd);
    return task_fd;
}

static fd_t open_file(Partition *part, u32 nr, FileFlag flags)
{
    fd_t fd = get_free_global_fd();
    if (fd == FILE_NULL)
        return fd;
    File *file = get_global_file(fd);
    bool success = onix_file_open(part, file, nr, flags);
    if (!success)
    {
        printk("open file failure!!!\n");
        return FILE_NULL;
    }
    Task *task = running_task();
    fd_t task_fd = task_install_fd(fd);
    return task_fd;
}

fd_t onix_sys_open(const char *pathname, FileFlag flags)
{
    fd_t fd = FILE_NULL;

    SearchRecord *record = malloc(sizeof(SearchRecord));
    memset(record, 0, sizeof(SearchRecord));

    Partition *part = get_path_part(pathname);

    u32 depth = path_depth(pathname);

    int nr = onix_search_file(pathname, record);

    bool found = nr != FILE_NULL ? true : false;

    if (record->type == FILETYPE_DIRECTORY)
    {
        printk("can`t open a direcotry with open(), use opendir() to instead\n");
        onix_dir_close(part, record->parent);
        return FILE_NULL;
    }

    // PBMB;
    u32 search_depth = path_depth(record->search_path);

    if (depth != search_depth)
    {
        printk("can`t access %s, subpath %s not exists\n", pathname, record->search_path);
        onix_dir_close(part, record->parent);
        return FILE_NULL;
    }

    if (!found && !(flags & O_C))
    {
        printk("file %s not exists\n", pathname);
        onix_dir_close(part, record->parent);
        return FILE_NULL;
    }
    // if (found && (flags & O_C))
    // {
    //     printk("file %s already exists\n", pathname);
    //     onix_dir_close(part, record->parent);
    //     return FILE_NULL;
    // }
    if (!found && (flags & O_C))
    {
        char name[MAX_FILENAME_LENGTH];
        memset(name, 0, sizeof(name));
        dirname(record->search_path, name);
        fd = create_file(part, record->parent, name, flags);
        onix_dir_close(part, record->parent);
        return fd;
    }
    else
    {
        fd = open_file(part, nr, flags);
        return fd;
    }
}

fd_t onix_sys_close(fd_t fd)
{
    fd_t ret = FILE_NULL;
    if (fd < 3)
        return ret;
    u32 global_fd = task_global_fd(fd);
    OnixFile *file = get_global_file(global_fd);
    bool success = onix_file_close(file);
    Task *task = running_task();
    task->file_table[fd] = FILE_NULL;
    return success;
}

int32 onix_sys_write(fd_t fd, const void *buf, u32 count)
{
    assert(fd >= 0 && fd < TASK_MAX_OPEN_FILES);
    u32 global_fd = task_global_fd(fd);
    OnixFile *file = get_global_file(global_fd);

    if (file->flags & O_W || file->flags & O_RW)
    {
        return onix_file_write(file, buf, count);
    }
    printk("file cannot write...\n");
    return 0;
}

int32 onix_sys_read(fd_t fd, void *buf, u32 count)
{
    assert(fd >= 0 && fd < TASK_MAX_OPEN_FILES);
    u32 global_fd = task_global_fd(fd);
    OnixFile *file = get_global_file(global_fd);
    return onix_file_read(file, buf, count);
}

int32 onix_sys_lseek(fd_t fd, int32 offset, Whence whence)
{
    assert(fd >= 0 && fd < TASK_MAX_OPEN_FILES);
    u32 global_fd = task_global_fd(fd);
    OnixFile *file = get_global_file(global_fd);
    return onix_file_lseek(file, offset, whence);
}

extern File file_table[MAX_OPEN_FILES];

int32 onix_sys_unlink(const char *pathname)
{
    assert(strlen(pathname) < MAX_PATH_LEN);

    SearchRecord *record = malloc(sizeof(SearchRecord));
    int step = 1;
    int32 ret = -1;
    if (record == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(record, 0, sizeof(SearchRecord));
    Partition *part = get_path_part(pathname);

    int nr = onix_search_file(pathname, record);
    if (nr == FILE_NULL)
    {
        printk("file %s not found!\n", pathname);
        step = 2;
        goto rollback;
    }
    if (record->type == FILETYPE_DIRECTORY)
    {
        printk("file %s is directory use rmdir instead!\n", pathname);
        onix_dir_close(part, record->parent);
        step = 2;
        goto rollback;
    }
    OnixFile *file = NULL;
    u32 idx = 0;
    while (idx < MAX_OPEN_FILES)
    {
        file = file_table + idx;
        if (file->inode != NULL && file->inode->nr == nr)
        {
            break;
        }
        idx++;
    }
    if (idx < MAX_OPEN_FILES)
    {
        step = 2;
        printk("file %s is in use\n", pathname);
        onix_dir_close(part, record->parent);
        goto rollback;
    }
    assert(idx == MAX_OPEN_FILES);

    Dir *parent = record->parent;
    onix_delete_dir_entry(part, parent, &record->entry);
    onix_inode_delete(part, nr);
    onix_dir_close(part, record->parent);
    ret = 0;
    step = 2;

rollback:
    switch (step)
    {
    case 2:
        free(record);
    case 1:
        break;
    default:
        break;
    }
    return ret;
}

int32 onix_sys_mkdir(const char *pathname)
{
    int32 success = -1;
    int step = 0;

    void *buf = malloc(BLOCK_SIZE);
    if (buf == NULL)
    {
        step = 1;
        goto rollback;
    }
    memset(buf, 0, BLOCK_SIZE);

    SearchRecord *record = malloc(sizeof(SearchRecord));
    if (record == NULL)
    {
        step = 2;
        goto rollback;
    }
    memset(record, 0, sizeof(SearchRecord));

    int nr = onix_search_file(pathname, record);
    if (nr != FILE_NULL)
    {
        step = 3;
        goto rollback;
    }

    Partition *part = get_path_part(pathname);

    int depth = path_depth(pathname);
    int search_path = path_depth(record->search_path);
    if (depth != search_path)
    {
        step = 4;
        goto rollback;
    }

    Dir *parent = record->parent;
    char *name = malloc(MAX_FILENAME_LENGTH);
    if (name == NULL)
    {
        step = 4;
        goto rollback;
    }

    memset(name, 0, MAX_FILENAME_LENGTH);

    basename(record->search_path, name);

    nr = onix_inode_bitmap_alloc(part);
    if (nr == -1)
    {
        step = 5;
        goto rollback;
    }

    Inode *inode = malloc(sizeof(Inode));
    if (inode == NULL)
    {
        step = 6;
        goto rollback;
    }

    memset(inode, 0, sizeof(Inode));

    onix_inode_init(nr, inode);

    u32 idx = onix_block_bitmap_alloc(part);
    if (idx == -1)
    {
        step = 7;
        goto rollback;
    }

    DirEntry holder;
    DirEntry *entry = &holder;
    memset(entry, 0, sizeof(holder));

    onix_init_dir_entry(name, nr, FILETYPE_DIRECTORY, entry);

    memset(buf, 0, BLOCK_SIZE);
    if (!onix_sync_dir_entry(part, parent, entry))
    {
        step = 8;
        goto rollback;
    }

    inode->blocks[0] = idx;
    onix_bitmap_sync(part, idx, BLOCK_BITMAP);

    entry = buf;

    memcpy(entry->filename, ".", 1);
    entry->inode_nr = nr;
    entry->type = FILETYPE_DIRECTORY;

    entry++;

    memcpy(entry->filename, "..", 2);
    entry->inode_nr = parent->inode->nr;
    entry->type = FILETYPE_DIRECTORY;

    partition_write(part, onix_block_lba(part, idx), buf, BLOCK_SECTOR_COUNT);

    inode->size = part->super_block->dir_entry_size * 2;

    onix_inode_sync(part, inode);

    onix_inode_sync(part, parent->inode);

    onix_bitmap_sync(part, nr, INODE_BITMAP);

    success = 0;
    free(inode);
    step = 5;

rollback:
    switch (step)
    {
    case 8:
        onix_block_bitmap_rollback(part, idx);
    case 7:
        free(inode);
    case 6:
        onix_inode_bitmap_rollback(part, nr);
    case 5:
        free(name);
    case 4:
        if (record->parent)
        {
            onix_dir_close(part, record->parent);
        }
    case 3:
        free(record);
    case 2:
        free(buf);
    case 1:
        break;
    default:
        break;
    }
    return success;
}

Dir *onix_sys_opendir(const char *pathname)
{
    int step = 0;
    Dir *dir = NULL;

    char *name = malloc(MAX_PATH_LEN);
    if (name == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(name, 0, MAX_PATH_LEN);
    abspath(pathname, name);
    if (!strcmp(name, "/"))
    {
        dir = &root_dir;
        step = 2;
        goto rollback;
    }

    Partition *part = get_path_part(pathname);
    SearchRecord *record = malloc(sizeof(SearchRecord));
    if (record == NULL)
    {
        step = 2;
        goto rollback;
    }
    memset(record, 0, sizeof(SearchRecord));

    int nr = onix_search_file(name, record);
    if (nr == FILE_NULL)
    {
        step = 3;
        goto rollback;
    }

    if (record->type == FILETYPE_REGULAR)
    {
        printk("%s is file!!!\n", pathname);
        step = 3;
        goto rollback;
    }

    if (record->type == FILETYPE_DIRECTORY)
    {
        dir = onix_dir_open(part, nr);
        dir->part = part;
        step = 3;
        goto rollback;
    }

    assert(false); // should never here....

rollback:
    switch (step)
    {
    case 3:
        if (record->parent != NULL)
        {
            onix_dir_close(part, record->parent);
        }
        free(record);
    case 2:
        free(name);
    case 1:
        break;
    default:
        break;
    }
    return dir;
}

int32 onix_sys_closedir(Dir *dir)
{
    int32 ret = -1;
    if (dir != NULL)
    {
        onix_dir_close(dir->part, dir);
        ret = 0;
    }
    return ret;
}

int32 onix_sys_rmdir(const char *pathname)
{
    int step = 0;
    int ret = -1;

    char *name = malloc(MAX_PATH_LEN);
    if (name == NULL)
    {
        step = 1;
        goto rollback;
    }

    memset(name, 0, MAX_PATH_LEN);
    abspath(pathname, name);
    if (!strcmp(name, "/"))
    {
        printk("cannot delete root dir\n");
        step = 2;
        goto rollback;
    }

    Partition *part = get_path_part(name);
    SearchRecord *record = malloc(sizeof(SearchRecord));
    if (record == NULL)
    {
        step = 2;
        goto rollback;
    }
    memset(record, 0, sizeof(SearchRecord));

    int nr = onix_search_file(name, record);
    if (nr == FILE_NULL)
    {
        printk("dir %s not exists!\n", pathname);
        step = 3;
        goto rollback;
    }

    if (record->type == FILETYPE_REGULAR)
    {
        printk("%s not directory!\n", pathname);
        step = 3;
        goto rollback;
    }

    assert(record->type == FILETYPE_DIRECTORY);

    Dir *dir = onix_dir_open(part, nr);
    // DEBUGP("dir size %lsd\n", dir->inode->size);
    if (!onix_dir_empty(part, dir))
    {
        printk("%s not empty!\n", pathname);
        step = 4;
        goto rollback;
    }

    if (!onix_dir_remove(part, record->parent, dir, &record->entry))
    {
        ret = 0;
    }
    step = 4;
rollback:
    switch (step)
    {
    case 4:
        onix_dir_close(part, dir);
    case 3:
        if (record->parent != NULL)
        {
            onix_dir_close(part, record->parent);
        }
        free(record);
    case 2:
        free(name);
    case 1:
        break;
    default:
        break;
    }
    return ret;
}

DirEntry *onix_sys_readdir(Dir *dir)
{
    assert(dir != NULL);
    return onix_dir_read(dir->part, dir);
}

void onix_sys_rewinddir(Dir *dir)
{
    dir->offset = 0;
}

int32 onix_sys_stat(const char *pathname, Stat *stat)
{
    char *name = malloc(MAX_PATH_LEN);
    int32 ret = -1;
    int step = 0;
    if (name == NULL)
    {
        step = 1;
        goto rollback;
    }
    memset(name, 0, MAX_PATH_LEN);
    abspath(pathname, name);
    Partition *part = get_path_part(name);

    if (!strcmp(name, "/"))
    {
        stat->nr = 0;
        stat->size = root_dir.inode->size;
        stat->type = FILETYPE_DIRECTORY;
        step = 2;
        ret = 0;
        goto rollback;
    }

    SearchRecord *record = malloc(sizeof(SearchRecord));
    if (record == NULL)
    {
        step = 2;
        goto rollback;
    }

    memset(record, 0, sizeof(SearchRecord));
    int nr = onix_search_file(name, record);
    if (nr == FILE_NULL)
    {
        step = 3;
        goto rollback;
    }

    Inode *inode = onix_inode_open(part, nr);
    stat->size = inode->size;
    stat->type = record->type;
    stat->nr = nr;
    ret = 0;
    step = 3;

rollback:
    switch (step)
    {
    case 3:
        if (record->parent != NULL)
        {
            onix_dir_close(part, record->parent);
        }
        free(record);
    case 2:
        free(name);
    case 1:
        break;
    default:
        break;
    }
    return ret;
}

void onix_list_dir(Dir *dir)
{
    DEBUGP("-----------0x%X-----------\n", dir->inode);
    DirEntry *entry = NULL;
    char *subpath = malloc(MAX_PATH_LEN);
    memset(subpath, 0, MAX_PATH_LEN);
    onix_sys_rewinddir(dir);
    while (true)
    {
        entry = onix_sys_readdir(dir);
        if (entry == NULL)
            break;
        basename(entry->filename, subpath);
        DEBUGP("%s \n", subpath);
    }
    free(subpath);
}