#ifndef ONIX_PATH_H
#define ONIX_PATH_H 1

#ifdef ONIX_KERNEL_DEBUG
#define dirname _dirname
#define is_split _is_split // avoid stand library
#define path_depth _path_depth
#endif

#include <onix/types.h>

bool is_split(char ch);
char *dirname(char *path, char *name);
u32 path_depth(char *path);

#endif