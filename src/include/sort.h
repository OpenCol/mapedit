/* sort.h -- two small insertion sorts, used by the palette code.
 *
 * Defined in sys/sort.c. sort_insertion orders n 32-bit keys ascending and
 * sort_insertion_8 n unsigned byte keys; each moves a payload array along with its keys.
 */
#ifndef MAPEDIT_SORT_H
#define MAPEDIT_SORT_H

#include "msc.h"

void sort_insertion(i16 n, i16 *vals, i32 *keys);
void sort_insertion_8(i16 n, char *payload, u8 *key);

#endif
