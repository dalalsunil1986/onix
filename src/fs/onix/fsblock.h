#ifndef ONIX_FSBLOCK_H
#define ONIX_FSBLOCK_H

#include <onix/types.h>
#include <onix/kernel/harddisk.h>

u32 get_block_lba(Partition *part, u32 idx);

#endif
