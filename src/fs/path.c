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
    int length = strlen(path);
    char *ptr1 = strrchr(path, '/');
    if (ptr1 == path || ptr1 == NULL)
    {
        memcpy(name, path, length);
        return name;
    }
    memcpy(name, path + 1, length - 1);
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