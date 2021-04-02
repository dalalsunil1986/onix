#include <fs/path.h>

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

