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
#include <stdlib.h>
#include <string.h>

#include "msc.h"

#define CONVENTIONAL_FREE 0x90000L     /* 576 KB free: a 640 KB machine after DOS and the EXE */

typedef struct Block {
    char tag[8];
    size_t size;
    char pad[16 - sizeof(size_t) % 16];
} Block;

extern char mem_last_alloc_failed;
extern i16 mem_manager_active;
extern void (*mem_manager_update)(void);

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
