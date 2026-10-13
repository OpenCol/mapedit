/* Imported from mapedit-decomp src/editor/strings by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE strings: MAPEDIT.EXE segment 0334 (strings.obj), file 0x494a-0x49de, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn strings_init   0x0494a
 * fn strings_shrink 0x0496c
 * fn strings_store  0x04976
 * fn strings        0x049ac
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0000). The CodeView names: the pool is strings_heap (DS:0x4a02), the
 * count num_strings (DS:0x6040), and the arena calls are heap_create / heap_shrink (heap_1.c).
 *
 * THE STRING POOL (root code, frame 0000: the first module of the program). DOS keeps the pool in an ovl31 arena (strings_heap, kind 9,
 * tag "$STRING") instead of a MemBlock:
 *   - strings_init(size) creates it and zeroes the count;
 *   - strings_shrink gives the arena's unused tail back to DOS (heap_shrink). Windows has
 *     strings_shutdown here instead;
 *   - strings_store(s) copies s into the next strlen + 1 bytes and returns its index;
 *   - strings(n) walks n NUL-terminated strings from the pool's base (inline repne scasb).
 *     Unlike Windows, it has no bounds check.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct {
    char kind;
    char owned;
    char *base;
    char *next;
    i32 size;
    i32 left;
} Arena;

extern i16 heap_create(i16 kind, Arena *a, i32 size, char *tag);
extern i32 heap_shrink(Arena *a);
extern char *heap_get(i32 n, Arena *a);
extern Arena strings_heap;              /* DS:0x4a02 */
extern i16 num_strings;                 /* DS:0x6040 */
/* strings_init -- 0334:000a, 34 bytes */
void strings_init(i32 size)
{
    heap_create(9, &strings_heap, size, "$STRING");
    num_strings = 0;
}
/* strings_shrink -- 0334:002c, 10 bytes */
void strings_shrink(void)
{
    heap_shrink(&strings_heap);
}
/* strings_store -- 0334:0036, 54 bytes */
// MSC: #pragma optimize("e", on)    /* strings_store keeps s in si (global register allocation) */
i16 strings_store(char *s)
{
    msc__fstrcpy(heap_get((i16)msc__fstrlen(s) + 1, &strings_heap), s);
    return num_strings++;
}
/* strings -- 0334:006c, 50 bytes */
/* PORT: strings is in port/ (inline assembly) */
char *strings(i16 n);


