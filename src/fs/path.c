#include <fs/path.h>
#include <fs/file.h>
#include <onix/string.h>
#include <onix/syscall.h>

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

char *abspath(char *path, char *buf)
{
    char *bptr = buf;
    if (is_split(path[0]))
    {
        *bptr = '/';
        bptr++;
    }
    else
    {
        sys_getcwd(buf, MAX_PATH_LEN);
        bptr = buf + strlen(buf);
    }

    char name[MAX_FILENAME_LENGTH];
    char *ptr = path;

    while (true)
    {
        memset(name, 0, sizeof(name));
        ptr = dirname(ptr, name);
        if (!name[0])
            break;
        if (!strcmp(name, "."))
        {
            continue;
        }
        if (!strcmp(name, ".."))
        {
            bptr = strrchr(buf, '/');
            *bptr = 0;
            continue;
        }
        if (*(bptr - 1) != '/')
        {
            *bptr = '/';
            bptr++;
        }
        u32 len = strlen(name);
        memcpy(bptr, name, len);
        bptr += len;
    }
    return buf;
}

bool exists(char *path)
{
    Stat stat;
    int32 ret = sys_stat(path, &stat);
    return (ret != -1);
}