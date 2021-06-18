#include <onix/memory.h>
#include <onix/string.h>

onix::memory::ards::ards onix::memory::ards::ards_holder[MAX_ARDS_COUNT];
u32 onix::memory::ards::ards_count;
onix::memory::ards::ards *onix::memory::ards::ards_descriptor;

void onix::memory::ards::initialize()
{
    memcpy(ards_holder, ards_descriptor, ards_count * sizeof(ards));
    ards_descriptor = ards_holder;
    ards *ptr;
    for (size_t i = 0; i < ards_count; i++)
    {
        ptr = &ards_holder[i];
        // 只有 type == 1 时 内存有效
        if (ptr->type != 1)
            continue;
        if (ptr->size > ards_descriptor->size)
            ards_descriptor = ptr;
    }
}