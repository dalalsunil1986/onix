#ifndef ONIX_MEMORY_H
#define ONIX_MEMORY_H

#include <onix/types.h>

#define ARDS_SIZE 32
static const u8 ARDS_TYPE_MEMORY = 1;
static const u8 ARDS_TYPE_RESERVED = 2;

typedef struct ARDS
{
    u32 addr0;
    u32 addr1;
    u32 size0; // length
    u32 size1;
    u32 type;
} ARDS;

void init_memory();

extern u32 total_memory_bytes;
extern u32 ards_count;
extern ARDS ards_table[ARDS_SIZE];

#endif