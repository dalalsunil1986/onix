#include <fs/onix/fssyscall.h>
#include <fs/onix/fsdir.h>
#include <fs/onix/fsfile.h>
#include <fs/onix/inode.h>
#include <fs/path.h>
#include <onix/string.h>
#include <onix/malloc.h>
#include <onix/kernel/task.h>
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

    // PBMB;

    u32 depth = path_depth(pathname);

    // PBMB;

    int nr = onix_search_file(pathname, record);

    // PBMB;

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