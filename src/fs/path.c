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
    char *ptr = path - 1;
    while (is_split(*(++ptr)))
        ;

    while (!is_split(*ptr) && *ptr != 0)
    {
        *name++ = *ptr++;
    }
    return ptr;
}

char *basename(char *path, char *name)
{
    u32 length = strlen(path);
    bool split_flag = false;

    while (is_split(path[--length]))
        ;
    u32 last = length + 1;

    while (!is_split(path[--length]))
        ;
    u32 start = length + 1;
    u32 size = last - start;
    memcpy(name, path + start, size);
    name[size] = 0;
    return name;
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