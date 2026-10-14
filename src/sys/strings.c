/* strings.c -- the string store: names and labels from the text files, found by number.
 *
 * The strings are kept end to end, each with its NUL, in one arena (strings_heap, a memory
 * block named "$STRING"). strings_store adds one and returns its number, counting from 0;
 * strings(n) finds string n by walking the n before it. Nothing is ever removed. The editor
 * makes the store at startup (strings_init).
 */
#include <string.h>
#include "msc.h"
#pragma pack(2)   /* records on 2-byte boundaries, as in 1994: the file formats depend on it */
#include "popup.h"
#include "error.h"
#include "pack.h"
#include "pal.h"
#include "sprite.h"
#include "font.h"
#include "loader.h"
#include "mem.h"
#include "records.h"
#include "text.h"

/* Makes an empty store of `size` bytes (9 is the arena's number in error reports). */
void strings_init(i32 size)
{
    heap_create(9, &strings_heap, size, "$STRING");
    num_strings = 0;
}
/* Copies s into the store and returns its number. A full store ends the program with an
 * error report (heap_get). */
i16 strings_store(char *s)
{
    msc__fstrcpy(heap_get((i16)msc__fstrlen(s) + 1, &strings_heap), s);
    return num_strings++;
}

Arena strings_heap;                       /* the strings, end to end */
i16 num_strings;                          /* how many are stored */

/* String n of the store. There is no bounds check: an n past the last string walks on into
 * the arena's unused space. */
char *strings(i16 n)
{
    char *p = strings_heap.base;
    u16 k;
    for (k = (u16)n; k != 0; k--)
        p += strlen(p) + 1;
    return p;
}
