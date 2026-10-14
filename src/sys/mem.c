/* mem.c -- memory: named blocks, the arenas carved from them, the free-memory figures.
 *
 * mem_get_name hands out blocks of a given size, each with a tag of up to 8 characters
 * ("$sys$", "$pack$", ...) that mem_get_block_name reads back; mem_free returns them. After
 * every allocation and release it calls an optional hook, mem_manager_update. In 1994 these
 * blocks came from DOS's memory-control-block chain or from upper memory, and the tag lived in
 * the block's DOS header. The port has one flat heap: a block is a calloc'd Block header (the
 * tag and the size) followed by the caller's bytes, and the functions that walked the DOS chain
 * are stubs.
 *
 * The free-memory figures describe a 640 KB machine with no upper, expanded or extended
 * memory, on which the original keeps everything in conventional memory: CONVENTIONAL_FREE
 * bytes are always free. They appear only in error reports.
 *
 * An Arena (heap_create, heap_declare) is a bump allocator over one block: heap_get takes the
 * next n bytes from the front and nothing is freed until heap_destroy releases the whole
 * block; heap_shrink gives back the unused tail. An arena's kind is the module number its
 * errors are reported under.
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

/* The allocator's bookkeeping. In the port only mem_last_alloc_failed and the hook are kept
 * up to date; the free-memory minima and maxima keep their initial values. */
char mem_initialized = 0;               /* never set in the port */
char mem_max_free_set = 0;              /* mem_max_free has been seeded */
char umb_max_free_set = 0;              /* umb_max_free has been seeded */
char mem_last_alloc_failed = 0;         /* the last mem_get_name returned null */
i16 mem_last_alloc_loader = 11;         /* module number memory errors are reported under */
i32 mem_last_alloc_size = 0;           /* the last request's size, for the error report */
i32 mem_last_alloc_avail = 0;          /* conventional memory free at the last request */
i32 mem_last_alloc_umb = 0;            /* upper memory free at the last request */
i32 mem_max_free = -1L;                /* most conventional memory ever free */
i32 mem_min_free = 100000000L;         /* least conventional memory ever free */
i32 umb_max_free = -1L;                /* the same for upper memory */
i32 umb_min_free = 100000000L;
void (*mem_manager_update)(void) = 0;   /* called after each allocation and release */
i16 mem_manager_active = 0;             /* -1 while the hook runs */
/* mem_call_hook -- calls a hook passed as the two 16-bit halves of a 1994 far pointer, with
 * mem_manager_active raised while it runs. Unused: the port calls mem_manager_update through
 * call_hook below. Port: the call reads a function pointer from the first half's address,
 * which is only meaningful for a 16-bit far pointer. */
void mem_call_hook(u16 off, u16 seg)
{
    if (off | seg) {
        mem_manager_active = -1;
        (*(void (**)(void))&off)();
        mem_manager_active = 0;
    }
}
/* mem_get -- a block of size bytes tagged "$sys$"; null when there is no memory. */
void *mem_get(i32 size)
{
    return mem_get_name(size, "$sys$");
}
/* mem_save_free -- remembers the two low-water marks for mem_restore_free. */
void mem_save_free(void)
{
    mem_save_free_area = mem_min_free;
    umb_save_free_area = umb_min_free;
}
/* mem_restore_free -- brings the low-water marks back to what was saved, or to what is free
 * now if that is less. */
void mem_restore_free(void)
{
    mem_min_free = min(mem_save_free_area, mem_conv_get_avail());
    umb_min_free = min(umb_save_free_area, 0);   /* no upper memory */
}

/* The low-water marks mem_save_free saved. */
i32 mem_save_free_area;
i32 umb_save_free_area;

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
/* mem_set_video_mode -- does nothing, in 1994 too. */
void mem_set_video_mode(void)
{
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
        a->owned = 1;
        a->next = a->base;
        a->size = a->left = size;
        err = 0;
    }
    return err;
}
/* heap_declare -- makes a an arena over an existing block, which the arena does not own. */
void heap_declare(i16 kind, Arena *a, char *base, i32 size)
{
    a->owned = 0;
    a->kind = kind;
    a->base = a->next = base;
    a->size = a->left = size;
}
/* heap_destroy -- frees the block if the arena owns it and empties the arena. */
void heap_destroy(Arena *a)
{
    if (a->owned)
        mem_free(a->base);
    a->base = 0;
    a->size = a->left = 0;
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

/* call_hook -- calls mem_manager_update, if set, with mem_manager_active raised. */
static void call_hook(void)
{
    if (mem_manager_update) {
        mem_manager_active = -1;
        mem_manager_update();
        mem_manager_active = 0;
    }
}

/* block_of -- the header of the block whose bytes start at p. */
static Block *block_of(void *p)
{
    return (Block *)p - 1;
}

/* mem_alloc_paras, mem_check_chain -- in 1994, allocate from and check DOS's memory-control-
 * block chain. The port has no chain: stubs. */
u16 mem_alloc_paras(u16 paras) { (void)paras; return 0; }
void mem_check_chain(void) { }

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
    call_hook();
    return b + 1;
}

/* mem_get_block_name -- copies block's tag into dest (9 bytes, terminated). */
void mem_get_block_name(void *block, char *dest)
{
    memcpy(dest, block_of(block)->tag, 8);
    dest[8] = 0;
}

/* mem_free -- frees a block from mem_get_name and calls the hook. Returns 0, or -1 for null. */
i16 mem_free(void *p)
{
    if (!p)
        return -1;
    free(block_of(p));
    call_hook();
    return 0;
}

/* mem_adjust -- resizes block p in place to size bytes. Shrinking always works (the port keeps
 * the memory and records the new size); growing fails with -1. Returns 0 on success. */
i16 mem_adjust(i32 size, void *p)
{
    Block *b = block_of(p);
    if ((size_t)size > b->size)
        return -1;
    b->size = (size_t)size;
    return 0;
}

/* mem_conv_largest -- the largest free conventional block, in 16-byte paragraphs. */
u16 mem_conv_largest(void) { return (u16)(CONVENTIONAL_FREE >> 4); }

/* mem_conv_get_avail -- the same in bytes. */
i32 mem_conv_get_avail(void) { return CONVENTIONAL_FREE; }

/* mem_check_overflow -- in 1994, folded a far pointer whose offset had passed 8000h back by
 * 32 KB into its segment. Flat pointers need nothing: returns p. Unused. */
void *mem_check_overflow(void *p) { return p; }

/* heap_shrink -- gives back the arena's unused tail, unless fewer than 16 bytes are left, so
 * that the arena is exactly full. Returns the arena's size. */
i32 heap_shrink(Arena *a)
{
    if (a->left >= 0x10) {
        i32 used = a->size - a->left;
        mem_adjust(used, a->base);
        a->size = used;
        a->left = 0;
    }
    return a->size;
}
