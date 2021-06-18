/* (C) Copyright 2021 Steven;
* @author: Steven kangweibaby@163.com
* @date: 2021-06-18
*/

#include <onix/types.h>

namespace onix
{
    namespace memory
    {
        static const u8 MAX_ARDS_COUNT = 20;

        struct ards
        {
            u64 base;
            u64 size;
            u32 type;
        } _packed;

        extern ards ards_holder[MAX_ARDS_COUNT];

        EXTERN u32 ards_count;
        EXTERN ards *descriptor;

        void init_ards();
    }
}