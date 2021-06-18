/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-18
*/

#include <onix/types.h>
#include <onix/console.h>
#include <onix/memory.h>

namespace onix
{
    EXTERN
    {
        short gdt_ptr = 0;
        int main()
        {
            memory::ards::initialize();

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