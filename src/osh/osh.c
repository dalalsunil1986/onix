#include <onix/stdio.h>
#include <onix/syscall.h>
#include <onix/string.h>
#include <onix/assert.h>
#include <fs/file.h>
#include <fs/path.h>

#define MAX_CMD_LEN 256
#define MAX_ARG_NR 16

static char cwd[MAX_PATH_LEN];
static char dname[MAX_FILENAME_LENGTH];
static char cmd[MAX_CMD_LEN];
static char *argv[MAX_ARG_NR];

void print_prompt()
{
    printf("[steven@localhost %s]# ", dname);
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
    char test[] = "asdf asdf asdf   asdf   asdf asdf";
    int argc = cmd_parse(test, argv, ' ');

    memset(cmd, 0, sizeof(cmd));
    memset(cwd, 0, sizeof(cwd));
    memset(dname, 0, sizeof(dname));

    sys_getcwd(cwd, MAX_PATH_LEN);
    basename(cwd, dname);
    clear();

    while (true)
    {
        print_prompt();
        readline(cmd, sizeof(cmd));
        if (cmd[0] == 0)
        {
            continue;
        }
        argc = cmd_parse(cmd, argv, ' ');
        if (argc < 0 || argc >= MAX_ARG_NR)
        {
            continue;
        }
    }
    panic("shoud not be here\n");
    return 0;
}
