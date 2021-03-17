#include <stdio.h>
#include <stdarg.h>
#include <onix/types.h>

int vsprintfk(char *buf, const char *fmt, va_list args);

typedef struct Test
{
    u8 g : 1;
    u8 db : 1;
    u8 l : 1;
    u8 avl : 1;
    u8 type : 4;
} _packed Test;

static char buf[1024];

int printk(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsprintfk(buf, fmt, args);
    va_end(args);

    printf(buf);
    // int n = i;
    // while (n-- > 0)
    // {
    //     put_char(buf[i - n - 1]);
    // }
    return i;
}


int main()
{
    printf("Test is starting...\n");
    printf("Test size: %d\n", sizeof(Test));

    Test t;
    u8 *value = (u8 *)(&t);

    t.g = 1;
    t.type = 0b1101;

    printf("Test.g %d\n", t.g);
    printf("Test %d\n", *value);

    printk("Hello printf %d", 123);

    return 0;
}