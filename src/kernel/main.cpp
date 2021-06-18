#include <onix/types.h>
#include <onix/console.h>

namespace onix
{

    EXTERN
    {
        short ards_count = 0;
        short ards_buffer = 0;
        short gdt_ptr = 0;

        int main()
        {
            auto value = console::get_cursor();
            console::set_cursor(255);
            console::clear();

            char *message = "hello world\0";
            char *ptr = message;
            do
            {
                console::putchar(*ptr);
            } while (*(++ptr));
        }
    }
};