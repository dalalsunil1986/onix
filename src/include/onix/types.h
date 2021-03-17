#ifndef ONIX_TYPES_H
#define ONIX_TYPES_H

#define _packed __attribute__((packed))

#ifndef _BOOL
typedef unsigned char bool;
static const bool false = 0;
static const bool true = 1;
#endif

typedef unsigned int size_t;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

#endif