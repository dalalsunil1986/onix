#include <onix/types.h>

namespace onix
{
    namespace kernel
    {
        EXTERN
        {
            short ards_count = 0;
            short ards_buffer = 0;
            short gdt_ptr = 0;

            int main()
            {
                char *video = (char *)(0xb8000);
                *video = 'K';
                return 0;
            }
        }
    }
};