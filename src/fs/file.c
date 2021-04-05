#include <fs/file.h>
#include <onix/kernel/assert.h>
#include <onix/string.h>
#include <onix/kernel/debug.h>
#include <onix/kernel/task.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGK
#else
#define DEBUGP(fmt, args...)
#endif

File file_table[MAX_OPEN_FILES];

fd_t get_free_global_fd()
{
    fd_t idx = 3;
    while (idx < MAX_OPEN_FILES)
    {
        if (file_table[idx].inode == NULL)
        {
            break;
        }
        idx++;
    }
    if (idx == MAX_OPEN_FILES)
    {
        printk("Exceed max open files\n");
        return -1;
    }
    return idx;
}

File *get_global_file(fd_t fd)
{
    assert(fd >= 0 && fd < MAX_OPEN_FILES);
    &file_table[fd];
}

void init_file()
{
    DEBUGK("Init global files...\n");
    for (size_t i = 0; i < MAX_OPEN_FILES; i++)
    {
        File *file = file_table + i;
        file->flags = 0;
        file->inode = NULL;
        file->offset = 0;
    }
}