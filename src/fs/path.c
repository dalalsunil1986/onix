#include <fs/path.h>
#include <fs/file.h>
#include <onix/string.h>

bool is_split(char ch)
{
    return (ch == '/' || ch == '\\');
}

char *dirname(char *path, char *name)
{
    if (path[0] == 0)
        return NULL;
    if (is_split(path[0]))
    {
        while (is_split(*(++path)))
            ;
    }

    while (!is_split(*path) && *path != 0)
    {
        *name++ = *path++;
    }
    return path;
}

u32 path_depth(char *path)
{
    if (path == NULL || path[0] == 0)
    {
        return 0;
    }
    char *ptr = path - 1;
    u32 depth = 0;
    bool split_last = false;
    bool split_flag = false;
    while (*(++ptr))
    {
        split_flag = is_split(*ptr);
        if (split_flag && !split_last)
        {
            split_last = true;
            continue;
        }
        if (split_flag && split_last)
        {
            continue;
        }
        if (!split_flag && split_last)
        {
            depth++;
        }
        split_last = false;
    }
    return depth;
}