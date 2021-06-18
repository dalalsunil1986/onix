#include <onix/types.h>
#include <onix/kernel/io.h>

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5

#define CRT_CURSOR_HIGH 0x0E
#define CRT_CURSOR_LOW 0x0F

namespace onix
{
    namespace kernel
    {
        EXTERN
        {
            short ards_count = 0;
            short ards_buffer = 0;
            short gdt_ptr = 0;

            u16 get_cursor()
            {
                u16 value = 0;
                outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
                value |= inb(CRT_DATA_REG) << 8;
                outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
                value |= inb(CRT_DATA_REG);
            }

            void set_cursor(u16 cursor)
            {
                u16 value = 0;
                outb(CRT_ADDR_REG, CRT_CURSOR_HIGH);
                outb(CRT_DATA_REG, cursor >> 8);
                outb(CRT_ADDR_REG, CRT_CURSOR_LOW);
                outb(CRT_DATA_REG, cursor & 0xff);
            }

            int main()
            {
                char *video = (char *)(0xb8000);
                *video = 'K';

                auto value = get_cursor();

                set_cursor(255);
            }
        }
    }
};