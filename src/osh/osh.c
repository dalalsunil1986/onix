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

#ifdef MAIN_DEBUG
int main(int argc, char const *argv[])
#else
int osh_task(int argc, char const *argv[])
#endif
{
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
    }
    return 0;
}
