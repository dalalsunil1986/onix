#include <onix/stdio.h>
#include <onix/syscall.h>
#include <onix/string.h>
#include <onix/assert.h>
#include <fs/file.h>
#include <fs/path.h>
#include <onix/malloc.h>

#define DEBUGINFO

#ifdef DEBUGINFO
#define DEBUGP DEBUGF
#else
#define DEBUGP(fmt, args...)
#endif

#define MAX_CMD_LEN 256
#define MAX_ARG_NR 16

static char cwd[MAX_PATH_LEN];
static char path[MAX_PATH_LEN];
static char subpath[MAX_PATH_LEN];
static char dname[MAX_FILENAME_LENGTH];
static char cmd[MAX_CMD_LEN];
static char *argv[MAX_ARG_NR];

void print_prompt()
{
    sys_getcwd(cwd, MAX_PATH_LEN);
    basename(cwd, dname);
    printf("[steven@localhost %s]# ", dname);
}

void buildin_pwd()
{
    printf("%s\n", sys_getcwd(cwd, MAX_PATH_LEN));
}

void buildin_clear()
{
    clear();
}

void buildin_ls(int argc, char *argv[])
{
    if (argc == 1)
    {
        sys_getcwd(path, MAX_PATH_LEN);
    }
    else
    {
        abspath(argv[1], path);
    }

    Stat stat;
    int ret = sys_stat(path, &stat);
    if (ret != 0)
    {
        printf("%s is not exists...\n", path);
        return;
    }
    if (stat.type == FILETYPE_DIRECTORY)
    {
        // DEBUGP("%s is dir...\n", path);
        Dir *dir = sys_opendir(path);
        DirEntry *entry = NULL;
        sys_rewinddir(dir);
        while (true)
        {
            entry = sys_readdir(dir);
            if (entry == NULL)
                break;
            basename(entry->filename, subpath);
            printf("%s ", subpath);
        }
        printf("\n");
    }
    else if (stat.type == FILETYPE_REGULAR)
    {
        // DEBUGP("%s is file...\n", path);
        printf("%s\n", path);
    }
}

void buildin_mkdir(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("mkdir: only support 1 argument!\n");
        return;
    }
    abspath(argv[1], path);

    if (!strcmp(path, "/"))
    {
        return;
    }
    if (exists(path))
    {
        DEBUGP("mkdir %s exists...\n", path);
        return;
    }
    return sys_mkdir(path);
}

void buildin_cd(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("mkdir: only support 1 argument!\n");
        return;
    }
    abspath(argv[1], path);
    return sys_chdir(path);
}

void buildin_test(int argc, char *argv[])
{
    // printf("test...\n");
    // void *buf = malloc(4);
    // free(buf);
    sys_test();
}

static void execute(int argc, char *argv[])
{
    char *line = argv[0];
    if (!strcmp(line, "pwd"))
    {
        return buildin_pwd();
    }
    if (!strcmp(line, "clear"))
    {
        return buildin_clear();
    }
    if (!strcmp(line, "ls"))
    {
        return buildin_ls(argc, argv);
    }
    if (!strcmp(line, "mkdir"))
    {
        return buildin_mkdir(argc, argv);
    }
    if (!strcmp(line, "cd"))
    {
        return buildin_cd(argc, argv);
    }
    if (!strcmp(line, "test"))
    {
        return buildin_test(argc, argv);
    }
}

void readline(char *buf, u32 count)
{
    assert(buf != NULL);
    char *ptr = buf;
    u32 idx = 0;
    while (idx < count)
    {
        ptr = buf + idx;
        int ret = sys_read(onix_stdin, ptr, 1);
        if (ret == -1)
        {
            *ptr = 0;
            return;
        }
        switch (*ptr)
        {
        case '\n':
        case '\r':
            *ptr = 0;
            sys_putchar('\n');
            return;
        case '\b':
            if (buf[0] != '\b')
            {
                idx--;
                sys_putchar('\b');
            }
            break;
        default:
            sys_putchar(*ptr);
            idx++;
            break;
        }
    }
}

static int cmd_parse(char *cmd, char *argv[], char token)
{
    assert(cmd != NULL);
    int idx = 0;
    while (idx < MAX_ARG_NR)
    {
        argv[idx] = NULL;
        idx++;
    }
    char *next = cmd;
    int32 argc = 0;
    while (*next && argc < MAX_ARG_NR)
    {
        while (*next == token)
        {
            next++;
        }

        if (*next == 0)
        {
            break;
        }
        argv[argc++] = next;
        while (*next && *next != token)
        {
            next++;
        }
        if (*next)
        {
            *next = 0;
            next++;
        }
    }
    return argc;
}

#ifdef MAIN_DEBUG
int main()
#else
int osh_task()
#endif
{
    memset(cmd, 0, sizeof(cmd));
    memset(cwd, 0, sizeof(cwd));
    memset(dname, 0, sizeof(dname));

    clear();

    while (true)
    {
        print_prompt();
        readline(cmd, sizeof(cmd));
        if (cmd[0] == 0)
        {
            continue;
        }
        int argc = cmd_parse(cmd, argv, ' ');
        if (argc < 0 || argc >= MAX_ARG_NR)
        {
            continue;
        }
        execute(argc, argv);
    }
    panic("shoud not be here\n");
    return 0;
}
