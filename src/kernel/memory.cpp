#include <onix/memory.h>
#include <onix/string.h>

onix::memory::ards onix::memory::ards_holder[MAX_ARDS_COUNT];
u32 onix::memory::ards_count;
onix::memory::ards *onix::memory::descriptor;

void onix::memory::init_ards()
{
    memcpy(ards_holder, descriptor, ards_count * sizeof(ards));
    descriptor = ards_holder;
    ards *ptr;
    for (size_t i = 0; i < ards_count; i++)
    {
        ptr = &ards_holder[i];
        // 只有 type == 1 时 内存有效
        if (ptr->type != 1)
            continue;
        if (ptr->size > descriptor->size)
            descriptor = ptr;
    }
}