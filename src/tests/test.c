#include <stdio.h>
#include <onix/types.h>

typedef struct Test
{
    u8 g : 1;
    u8 db : 1;
    u8 l : 1;
    u8 avl : 1;
    u8 type : 4;
} _packed Test;

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
    return 0;
}