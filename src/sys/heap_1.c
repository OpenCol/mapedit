/* From mapedit-decomp src/madsdev/heap_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "error.h"
#include "loader.h"
#include "mem.h"
#include "records.h"
#include "units.h"
/* MODULE heap_1: MAPEDIT.EXE segment 0f45 (heap_1.c), file 0x10a50-0x10c50, 5 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn heap_create  0x10a50
 * fn heap_declare 0x10ae0
 * fn heap_destroy 0x10b1e
 * fn heap_get     0x10b5a
 * fn heap_shrink  0x10bc4
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl31_78b3e).
 *
 * ARENAS (overlay 31, para 79). DOS-only engine code: a bump allocator carved out of one
 * far block. The functions are _fastcall (or callee-pops), with the arena on the stack.
 *   - heap_create(kind, a, size, tag) takes the block from mem_get_name and calls
 *     error_report(-60, 2, kind, size, mem_get_avail()) when it cannot.
 *   - heap_declare(kind, a, base, size) wraps an
 *     existing block, which the arena then does not own.
 *   - heap_destroy frees the block if the arena owns it, then zeroes the arena.
 *   - heap_get(n, a) returns the next n bytes, and calls
 *     error_report(-61, 2, kind, n, left) when they do not fit.
 *   - heap_shrink gives the unused tail back to DOS (int 21h, AH=4Ah) when at least 16
 *     bytes are left, and returns the arena's size.
 *
 * heap_shrink needs two odd spellings:
 *   - `used = ...` written twice. With a single assignment, MSC sinks it into the paragraph
 *     count and hoists `base = a->base` above it, while shipped computes used first and
 *     copies the base through cx:si. The repeat stops the sink, as in ovl30's fades.
 *   - the test as `if (a->left < 0x10) ; else { ... }`. That gives the shipped `jg`-before-
 *     `jl` order of the long compare; `>= 0x10`, `> 15`, `16L`, `!(<)` and pads 0-16 all give
 *     `jl` first.
 */



/* heap_create -- 0f45:0000, 144 bytes */
i16 heap_create(i16 kind, Arena *a, i32 size, char *tag)
{
    i16 err;

    err = 1;
    a->kind = kind;
    a->base = mem_get_name(size, tag);
    if (a->base == 0)
        error_report(-60, 2, kind, size, mem_get_avail());
    else {
        a->owned = 1;
        a->next = a->base;
        a->size = a->left = size;
        err = 0;
    }
    return err;
}
/* heap_declare -- 0f45:0090, 62 bytes */
void heap_declare(i16 kind, Arena *a, char *base, i32 size)
{
    a->owned = 0;
    a->kind = kind;
    a->base = a->next = base;
    a->size = a->left = size;
}
/* heap_destroy -- 0f45:00ce, 60 bytes */
void heap_destroy(Arena *a)
{
    if (a->owned)
        mem_free(a->base);
    a->base = 0;
    a->size = a->left = 0;
}
/* heap_get -- 0f45:010a, 106 bytes */
void *heap_get(i32 n, Arena *a)
{
    char *r;

    r = 0;
    /* the menu and popup arenas (0x28, 0x29) hold records, which need an even address
       (/Zp2) where 1994's 8086 did not; the others keep 1994's layout -- the strings heap
       (9) is its strings end to end, and strings() finds the n-th by counting them */
    if ((u8)a->kind == 0x28 || (u8)a->kind == 0x29)
        n = (n + 1) & ~1L;
    if (a->left < n)
        error_report(-61, 2, (u8)a->kind, n, a->left);
    else {
        r = a->next;
        a->next += (u16)n;
        a->left -= n;
    }
    return r;
}
/* heap_shrink -- 0f45:0174, 140 bytes */
/* PORT: heap_shrink is in port/ (inline assembly) */


