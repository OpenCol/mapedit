/* mem.c -- memory: named blocks, the arenas carved from them, the free-memory figures.
 *
 * mem_get_name hands out blocks of a given size, each with a tag of up to 8 characters
 * ("$sys$", "$pack$", ...); mem_free returns them. In 1994 these blocks came from
 * DOS's memory-control-block chain or from upper memory, and the tag lived in the block's DOS
 * header. The port has one flat heap: a block is a calloc'd Block header (the tag and the
 * size) followed by the caller's bytes.
 *
 * The free-memory figures describe a 640 KB machine with no upper, expanded or extended
 * memory, on which the original keeps everything in conventional memory: CONVENTIONAL_FREE
 * bytes are always free. They appear only in error reports.
 *
 * An Arena (heap_create, heap_declare) is a bump allocator over one block: heap_get takes the
 * next n bytes from the front, and nothing is ever given back. An arena's kind is the module
 * number its errors are reported under.
 */
#include <stdlib.h>
#include <string.h>

#include "msc.h"
#include "error.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mem.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */

/* mem_normalize -- in 1994, a far pointer with its offset reduced to 0-15. Flat pointers need
 * nothing: returns p. */
char *mem_normalize(char *p)
{
    return p;
}

char mem_last_alloc_failed = 0;         /* the last mem_get_name returned null */
i16 mem_last_alloc_loader = 11;         /* a module number; the loaders set it, nothing reads it */
i32 mem_last_alloc_size = 0;           /* shown by the error report; never set, so 0 */
i32 mem_last_alloc_avail = 0;          /* shown by the error report; never set, so 0 */
/* mem_get -- a block of size bytes tagged "$sys$"; null when there is no memory. */
void *mem_get(i32 size)
{
    return mem_get_name(size, "$sys$");
}

/* mem_get_avail -- the larger of the free conventional and upper memory: always
 * CONVENTIONAL_FREE in the port. */
i32 mem_get_avail(void)
{
    i32 dos;
    i32 heap;

    dos = mem_conv_get_avail();
    heap = 0;                   /* no upper memory */
    return heap >= dos ? heap : dos;
}

/* heap_create -- makes a an arena over a new block of size bytes tagged tag, owned by the arena.
 * kind is the module number errors are reported under. Returns 0, or 1 when there is no memory
 * (after error_report, which ends the program). */
i16 heap_create(i16 kind, Arena *a, i32 size, char *tag)
{
    i16 err;

    err = 1;
    a->kind = kind;
    a->base = mem_get_name(size, tag);
    if (a->base == 0)
        error_report(-60, 2, kind, size, mem_get_avail());
    else {
    
        a->next = a->base;
        a->size = a->left = size;
        err = 0;
    }
    return err;
}
/* heap_declare -- makes a an arena over an existing block, which the arena does not own. */
void heap_declare(i16 kind, Arena *a, char *base, i32 size)
{
    a->kind = kind;
    a->base = a->next = base;
    a->size = a->left = size;
}
/* heap_get -- the next n bytes of the arena, or null after error_report (which ends the
 * program) when they do not fit. The pointer advances by n's low 16 bits, as in 1994, where
 * an arena never exceeded 64 KB. */
void *heap_get(i32 n, Arena *a)
{
    char *r;

    r = 0;
    /* Port: the menu and popup arenas (0x28, 0x29) hold records, which need an even address
     * on a modern machine where 1994's 8086 did not, so their sizes round up to even. The
     * others keep 1994's layout: the strings arena (9) holds its strings end to end, and
     * strings() finds the n-th by counting them. */
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
#define CONVENTIONAL_FREE 0x90000L     /* 576 KB free: a 640 KB machine after DOS and the EXE */

/* The header in front of every block. */
typedef struct Block {
    char tag[8];
    size_t size;
    char pad[16 - sizeof(size_t) % 16];
} Block;

/* block_of -- the header of the block whose bytes start at p. */
static Block *block_of(void *p)
{
    return (Block *)p - 1;
}

/* mem_set_block_name -- stores up to 8 characters of name as block's tag. */
void mem_set_block_name(void *block, char *name)
{
    Block *b = block_of(block);
    size_t i;
    memset(b->tag, 0, sizeof b->tag);
    for (i = 0; i < 8 && name && name[i]; i++)
        b->tag[i] = name[i];
}

/* mem_get_name -- a zeroed block of size bytes (a negative size counts as 0) tagged tag, with
 * 16 spare bytes after it; null when there is no memory. Sets mem_last_alloc_failed and calls
 * the hook on success. */
void *mem_get_name(i32 size, char *tag)
{
    Block *b;
    if (size < 0)
        size = 0;
    b = calloc(1, sizeof *b + (size_t)size + 16);
    mem_last_alloc_failed = b == NULL;
    if (!b)
        return NULL;
    b->size = (size_t)size;
    mem_set_block_name(b + 1, tag);
    return b + 1;
}

/* mem_free -- frees a block from mem_get_name and calls the hook. Returns 0, or -1 for null. */
i16 mem_free(void *p)
{
    if (!p)
        return -1;
    free(block_of(p));
    return 0;
}

/* mem_conv_get_avail -- the conventional memory free, in bytes: always CONVENTIONAL_FREE. */
i32 mem_conv_get_avail(void) { return CONVENTIONAL_FREE; }

