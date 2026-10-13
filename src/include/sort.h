/* sort.h -- the functions, data and records of misc.c, sort_1.c. */
#ifndef MAPEDIT_SORT_H
#define MAPEDIT_SORT_H

#include "msc.h"

void sort_insertion(i16 n, i16 *vals, i32 *keys);
void sort_insertion_8(i16 n, char *payload, u8 *key);

#endif
