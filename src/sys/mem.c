/* mem.c -- memory: named blocks, the arenas carved from them, the free-memory figures. */
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
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE mem_1: MAPEDIT.EXE segment 104f (MEM_1.C), file 0x11af2-0x11b0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mem_normalize 0x11af2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c05).
 *
 * FAR POINTER NORMALISATION (root code, frame 0c05; pascal). mem_normalize(p) moves all but
 * the low four bits of p's offset into its segment, so the offset is 0..15. Inline assembly.
 */

/* mem_normalize -- 104f:0002, 24 bytes */
/* PORT: mem_normalize is in port/ (inline assembly) */

/* mem_normalize -- 104f:0002: a far pointer with the smallest offset. Flat pointers are
   normal already. */
char *mem_normalize(char *p)
{
    return p;
}

/* MODULE mem_2: MAPEDIT.EXE segment 0cc9 (mem_2.c), file 0xe294-0xe68e, 11 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mem_alloc_paras      0x0e294
 * fn mem_check_chain      0x0e32c
 * fn mem_call_hook        0x0e380
 * fn mem_set_block_name   0x0e39c
 * fn mem_get_name         0x0e3c6
 * fn mem_get              0x0e572
 * fn mem_get_block_name   0x0e57c
 * fn mem_free             0x0e5a0
 * fn mem_adjust           0x0e5f0
 * fn mem_save_free        0x0e61e
 * fn mem_restore_free     0x0e63c
 *
 * THE LIBRARY'S FAR-MEMORY MANAGER (madsdev.lib). Every block comes either from upper
 * memory through XMS (xms_umb_get allocates, xms_umb_free frees, xms_umb_get_avail reports
 * the largest free one) or from conventional memory, which this module carves out of the
 * DOS memory-control-block chain itself (not through int 21h AH=48h):
 *   - mem_alloc_paras(paras) (static) walks the MCB chain from the list of lists (AH=52h),
 *     merging adjacent free blocks as it goes, and takes the first free block that fits:
 *     an exact fit is claimed as is, a bigger one is split (the remainder gets a new 'M'
 *     header). The block's owner becomes the PSP (AH=62h); it returns the data segment
 *     (header + 1) or 0 when the chain ends ('Z') without a fit.
 *   - mem_check_chain() (static) walks the chain again and reports the first header that
 *     is neither 'M' nor 'Z' as error_report(-66, 3, 11, segment, 0) -- the memory arena
 *     is corrupt.
 *   - mem_call_hook(off, seg) (static) is VICEROY's call_guarded: it calls the far hook, if
 *     any, with mem_manager_active raised. The pointer arrives as two words, the only
 *     spelling that tests the offset first.
 *   - mem_set_block_name(block, name) (static) writes up to 8 characters of the tag into
 *     the block's MCB owner-name field (DOS 4+, header offset 8).
 *   - mem_get_name(size, tag) is VICEROY's far_alloc_tagged: records the request and what
 *     conventional memory and the UMBs have free, seeds the low-water marks the first time,
 *     takes the block from the UMBs when it fits, otherwise from the MCB chain; on a
 *     conventional failure it checks the chain. It keeps the minima, sets
 *     mem_last_alloc_failed, names the block and calls the hook. The chain allocation and
 *     the check are called from an `_asm` block, which is why both are FAR calls with
 *     fixups (a C call to a function in the same file is `push cs; call near`).
 *   - mem_get is mem_get_name with the tag "$sys$".
 *   - mem_get_block_name(block, dest) (_fastcall, both on the stack) copies the owner name
 *     out of a block's MCB.
 *   - mem_free: a segment of 0xa000 and up is a UMB, anything lower goes back to DOS
 *     (AH=49h). Either way it then calls the hook.
 *   - mem_adjust resizes a DOS block (AH=4Ah).
 *   - mem_save_free and mem_restore_free save the two low-water marks and later bring them
 *     back down to the current free amounts.
 *
 * The module's data (DS:0x748-0x76f, CodeView names) is defined before the first function,
 * ahead of the literal "$sys$" (DS:0x770).
 */




char mem_initialized = 0;               /* DS:0x748 */
char mem_max_free_set = 0;              /* DS:0x749 */
char umb_max_free_set = 0;              /* DS:0x74a */
char mem_last_alloc_failed = 0;         /* DS:0x74b */
i16 mem_last_alloc_loader = 11;         /* DS:0x74c */
i32 mem_last_alloc_size = 0;           /* DS:0x74e */
i32 mem_last_alloc_avail = 0;          /* DS:0x752 */
i32 mem_last_alloc_umb = 0;            /* DS:0x756 */
i32 mem_max_free = -1L;                /* DS:0x75a */
i32 mem_min_free = 100000000L;         /* DS:0x75e */
i32 umb_max_free = -1L;                /* DS:0x762 */
i32 umb_min_free = 100000000L;         /* DS:0x766 */
void (*mem_manager_update)(void) = 0;   /* DS:0x76a */
i16 mem_manager_active = 0;             /* DS:0x76e */
/* mem_alloc_paras -- 0cc9:0004, 152 bytes */
/* PORT: mem_alloc_paras is in port/ (inline assembly) */

/* mem_check_chain -- 0cc9:009c, 84 bytes */
/* PORT: mem_check_chain is in port/ (inline assembly) */

/* mem_call_hook -- 0cc9:00f0, 28 bytes */
/* The hook arrives as its two words: shipped tests `off | seg` offset first, which no
 * single far-pointer symbol gives (MSC always loads the segment half first). The pads set
 * the two symbols' order. */
void mem_call_hook(u16 off, u16 seg)
{
    if (off | seg) {
        mem_manager_active = -1;
        (*(void (**)(void))&off)();
        mem_manager_active = 0;
    }
}
/* mem_set_block_name -- 0cc9:010c, 42 bytes */

/* PORT: mem_set_block_name is in port/ (inline assembly) */

/* mem_get_name -- 0cc9:0136, 428 bytes */
/* 13 pads: the heapfree-vs-size compares load heapfree (cx:bx) into ax:dx and test the
 * spilled size against it; 0-12 pads give the reverse. */
/* PORT: mem_get_name is in port/ (inline assembly) */

/* mem_get -- 0cc9:02e2, 10 bytes */
void *mem_get(i32 size)
{
    return mem_get_name(size, "$sys$");
}
/* mem_get_block_name -- 0cc9:02ec, 36 bytes */
/* PORT: mem_get_block_name is in port/ (inline assembly) */

/* mem_free -- 0cc9:0310, 80 bytes */
/* PORT: mem_free is in port/ (inline assembly) */

/* mem_adjust -- 0cc9:0360, 46 bytes */
/* PORT: mem_adjust is in port/ (inline assembly) */

/* mem_save_free -- 0cc9:038e, 30 bytes */
void mem_save_free(void)
{
    mem_save_free_area = mem_min_free;
    umb_save_free_area = umb_min_free;
}
/* mem_restore_free -- 0cc9:03ac, 82 bytes */
void mem_restore_free(void)
{
    mem_min_free = min(mem_save_free_area, mem_conv_get_avail());
    umb_min_free = min(umb_save_free_area, 0);   /* no upper memory */
}

/* The module's uninitialised data (communals, BSS). */
i32 mem_save_free_area;                  /* DS:0x4ae8 */
i32 umb_save_free_area;                  /* DS:0x51c8 */

/* MODULE mem_3: MAPEDIT.EXE segment 1357 (MEM_3.C), file 0x14b7e-0x14b96, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mem_check_overflow 0x14b7e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * mem_check_overflow(p) (_fastcall, the pointer on the stack) folds a far pointer whose
 * offset has crossed 0x8000 back by 32 KB (offset - 0x8000, segment + 0x800), the step the
 * blitters take between rows. Inline assembly; a C spelling would store its halves.
 */
/* mem_check_overflow -- 1357:000e, 24 bytes */
/* PORT: mem_check_overflow is in port/ (inline assembly) */

/* MODULE mem_4: MAPEDIT.EXE segment 0d08 (mem_4.c), file 0xe68e-0xe72c, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mem_conv_largest   0x0e68e
 * fn mem_conv_get_avail 0x0e6e8
 * fn mem_get_avail      0x0e702
 * fn mem_set_video_mode 0x0e72a
 *
 * HOW MUCH MEMORY IS FREE (madsdev.lib).
 *   - mem_conv_largest (static, `near`: a plain near call and `ret`) walks the DOS
 *     memory-control-block chain from the list of lists (int 21h, AH=52h), merging adjacent
 *     free blocks as mem_2's allocator does, and returns the largest free block in
 *     paragraphs. All inline assembly.
 *   - mem_conv_get_avail returns that in bytes: the paragraph count is scaled by 16 into
 *     dx:ax by hand (an `_asm` block after the C call, which is why it has a bp frame
 *     with no locals; `(unsigned long)n << 4` gives a shl/rcl chain).
 *   - mem_get_avail returns the larger of that and what the upper-memory heap reports
 *     (xms_umb_get_avail). It is the "Most recent memory check" of the error report.
 *   - mem_set_video_mode is an empty function.
 */


/* mem_conv_largest -- 0d08:000e, 90 bytes */
/* PORT: mem_conv_largest is in port/ (inline assembly) */

/* mem_conv_get_avail -- 0d08:0068, 26 bytes */
/* PORT: mem_conv_get_avail is in port/ (inline assembly) */

/* mem_get_avail -- 0d08:0082, 40 bytes */
i32 mem_get_avail(void)
{
    i32 dos;
    i32 heap;

    dos = mem_conv_get_avail();
    heap = 0;                   /* no upper memory */
    return heap >= dos ? heap : dos;
}
/* mem_set_video_mode -- 0d08:00aa, 2 bytes */
void mem_set_video_mode(void)
{
}

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

/* mem.c -- madsdev's memory managers (mem_2/3/4.c, heap_1.c's heap_shrink, the EMS and XMS
 * modules), whose work was DOS's: the MCB chain, INT 21h AH=48h/49h/4Ah, INT 67h, the XMS
 * driver. The port has one flat heap.
 *
 * mem_get_name keeps the library's interface: a block of `size` bytes with an 8-character
 * tag (the original wrote it into the block's MCB, where mem_get_block_name finds it), the
 * hook called after every allocation and release, and the bookkeeping the editor reports in
 * its memory check. Conventional memory is a full 640 KB machine's; there is no EMS, XMS or
 * upper memory, as on a machine without them -- the library then keeps everything in
 * conventional memory, which is what the port has plenty of.
 */


#define CONVENTIONAL_FREE 0x90000L     /* 576 KB free: a 640 KB machine after DOS and the EXE */

typedef struct Block {
    char tag[8];
    size_t size;
    char pad[16 - sizeof(size_t) % 16];
} Block;


static void call_hook(void)
{
    if (mem_manager_update) {
        mem_manager_active = -1;
        mem_manager_update();
        mem_manager_active = 0;
    }
}

static Block *block_of(void *p)
{
    return (Block *)p - 1;
}

/* mem_alloc_paras, mem_check_chain -- the MCB chain itself: the port has none */
u16 mem_alloc_paras(u16 paras) { (void)paras; return 0; }
void mem_check_chain(void) { }

/* mem_set_block_name -- 0cc9:010c: up to 8 characters of the tag */
void mem_set_block_name(void *block, char *name)
{
    Block *b = block_of(block);
    size_t i;
    memset(b->tag, 0, sizeof b->tag);
    for (i = 0; i < 8 && name && name[i]; i++)
        b->tag[i] = name[i];
}

/* mem_get_name -- 0cc9:0136: size bytes, tagged; NULL when there is no memory */
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

/* mem_get_block_name -- 0cc9:02ec: the tag, with its terminating zero */
void mem_get_block_name(void *block, char *dest)
{
    memcpy(dest, block_of(block)->tag, 8);
    dest[8] = 0;
}

/* mem_free -- 0cc9:0310: 0 when it worked */
i16 mem_free(void *p)
{
    if (!p)
        return -1;
    free(block_of(p));
    call_hook();
    return 0;
}

/* mem_adjust -- 0cc9:0360: resize a block in place (DOS AH=4Ah). Shrinking always works in
   place; the port keeps the memory and records the new size. */
i16 mem_adjust(i32 size, void *p)
{
    Block *b = block_of(p);
    if ((size_t)size > b->size)
        return -1;
    b->size = (size_t)size;
    return 0;
}

/* mem_conv_largest -- 0d08:000e: the largest free conventional block, in paragraphs */
u16 mem_conv_largest(void) { return (u16)(CONVENTIONAL_FREE >> 4); }

/* mem_conv_get_avail -- 0d08:0068: the same in bytes */
i32 mem_conv_get_avail(void) { return CONVENTIONAL_FREE; }

/* mem_check_overflow -- 1357:000e, mem_normalize's twin for offsets past 8000h */
void *mem_check_overflow(void *p) { return p; }

/* heap_shrink -- 0f45:0174: give the arena's unused tail back to DOS, unless under 16 bytes
   are left; returns the arena's size */
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
