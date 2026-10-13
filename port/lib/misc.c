/* misc.c -- small library routines whose bodies were inline assembly: the string table,
 * hot spots, console output, the packers' memory streams, a byte-keyed sort, a colour hash.
 * Each one does what its original does, quirks included; tests/oracle checks them against
 * MAPEDIT.EXE.
 */
#include <stdio.h>
#include <string.h>

#include "msc.h"
#include "loader.h"
#include "menu.h"
#include "popup.h"
#include "sprite.h"
#include "error.h"
#include "hspot.h"
#include "pack.h"
#include "pal.h"
#include "records.h"
#include "sort.h"
#include "text.h"
#include "units.h"


/* strings -- 0334:006c: the n-th of the NUL-separated strings in the string heap */
char *strings(i16 n)
{
    char *p = strings_heap.base;
    u16 k;
    for (k = (u16)n; k != 0; k--)
        p += strlen(p) + 1;
    return p;
}

/* hspot_which -- 0baa:0002: the first active hot spot (from 1) with this id whose rectangle
   holds (x, y), compared unsigned; 0 when none does */
i16 hspot_which(u16 x, u16 y, i16 id)
{
    u16 i;
    for (i = 1; i <= (u16)numspots; i++) {
        HotSpot *s = &spot[i];
        if (s->active == 0)
            continue;
        if (x < (u16)s->x0 || x > (u16)s->x1 || y < (u16)s->y0 || y > (u16)s->y1)
            continue;
        if (id == s->id)
            return (i16)i;
    }
    return 0;
}

/* echo -- 127b:0000: text to the console (DOS handle 1), and LF CR after it if kind != 0 */
void echo(i16 kind, char *text)
{
    fputs(text, stdout);
    if (kind)
        fputs("\n", stdout);
    fflush(stdout);
}

/* far_to_near -- 1309:000a: a far pointer's offset. The port's pointers have no segment;
   what callers hand on is the pointer's low bits, as the original's offset. */
u16 far_to_near(void *p)
{
    return (u16)(uintptr_t)p;
}

/* io_call_hook -- 1309:0014: the packers' special function, when there is one */
void io_call_hook(void)
{
    if (pack_special_function)
        pack_special_function();
}

/* pack_read_memory -- 1359:0006: the packers' reader over a block of memory. Takes *n bytes,
   no more than pack_read_size has left, counts them, returns how many. A size whose high
   word is FFFFh (any negative one) means no limit; only a size under 64 KB clamps. *n is not
   written back. */
u16 pack_read_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_read_size >> 16) != 0xffff) {
        if (pack_read_size == 0)
            return 0;
        if (((u32)pack_read_size >> 16) == 0 && c > (u16)pack_read_size)
            c = (u16)pack_read_size;
        pack_read_size -= c;
    }
    pack_read_count += c;
    if (c) {
        memcpy(buf, pack_read_memory_ptr, c);
        pack_read_memory_ptr += c;
    }
    return c;
}

/* pack_write_memory -- 1360:000a: the writer, the same way round. The original has no return
   statement and returns what mem_normalize left in AX, the new pointer's offset after
   normalising (0-15), which depends on where DOS put the block; PKWARE's library, which calls
   it, ignores a writer's result. The port returns the count. */
u16 pack_write_memory(void *data, u16 *n)
{
    char *buf = data;
    u16 c = *n;
    if (((u32)pack_write_size >> 16) != 0xffff) {
        if (pack_write_size == 0)
            return 0;
        if (((u32)pack_write_size >> 16) == 0 && c > (u16)pack_write_size)
            c = (u16)pack_write_size;
        pack_write_size -= c;
    }
    pack_write_count += c;
    if (c) {
        memcpy(pack_write_memory_ptr, buf, c);
        pack_write_memory_ptr += c;
    }
    return c;
}

/* sort_insertion_8 -- 12e0:0008: sort n byte keys ascending, carrying a payload byte each.
   Exactly the original's steps, which decide the order of equal keys: find the first i whose
   key[i+1] < key[i] (unsigned); lift key[i+1] and its payload out, closing the gap; put them
   back before the first key not below it; scan again from that i. */
void sort_insertion_8(i16 n, char *payload, u8 *key)
{
    i16 i = 0;
    for (;;) {
        i16 s = i, last = (i16)(n - 1), cnt, k;
        u8 al;
        char dl;
        while (s < last && !(key[s + 1] < key[s]))
            i = ++s;
        if (s >= last)
            return;
        al = key[s + 1];
        cnt = (i16)(last - s - 1);
        memmove(&key[s + 1], &key[s + 2], (size_t)cnt);
        dl = payload[s + 1];
        memmove(&payload[s + 1], &payload[s + 2], (size_t)cnt);
        for (k = 0; k < last && al > key[k]; k++)
            ;
        cnt = (i16)(last - k);
        if (cnt) {
            memmove(&key[k + 1], &key[k], (size_t)cnt);
            memmove(&payload[k + 1], &payload[k], (size_t)cnt);
        }
        payload[k] = dl;
        key[k] = al;
    }
}

/* magic_hash_color -- 133e:000a: 38 r + 76 g + 14 b, in 16 bits */
i16 magic_hash_color(const void *color)
{
    const u8 *rgb = color;
    return (i16)(u16)(38u * rgb[0] + 76u * rgb[1] + 14u * rgb[2]);
}
