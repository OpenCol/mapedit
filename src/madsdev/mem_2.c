/* Imported from mapedit-decomp src/madsdev/mem_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

#define min(a, b) ((a) < (b) ? (a) : (b))
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern i32 mem_conv_get_avail(void);                       /* 0d08:0068 */
extern i32 xms_umb_get_avail(void);                        /* 127e:000c */
extern void *xms_umb_get(i32 size);                    /* 1281:0008 */
extern i16 xms_umb_free(void *p);                       /* 1281:004a */
extern void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2); /* 0ed0:03d6 */
extern i32 mem_save_free_area;                             /* DS:0x4ae8 */
extern i32 umb_save_free_area;                             /* DS:0x51c8 */

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
u16 mem_alloc_paras(u16 paras);

/* mem_check_chain -- 0cc9:009c, 84 bytes */
/* PORT: mem_check_chain is in port/ (inline assembly) */
void mem_check_chain(void);

/* mem_call_hook -- 0cc9:00f0, 28 bytes */
/* The hook arrives as its two words: shipped tests `off | seg` offset first, which no
 * single far-pointer symbol gives (MSC always loads the segment half first). The pads set
 * the two symbols' order. */
extern i16 pad_cg0;
extern i16 pad_cg1;
void mem_call_hook(u16 off, u16 seg)
{
    if (off | seg) {
        mem_manager_active = -1;
        (*(void (**)(void))&off)();
        mem_manager_active = 0;
    }
}
/* mem_set_block_name -- 0cc9:010c, 42 bytes */
extern i16 pad_cg2;
extern i16 pad_cg3;
extern i16 pad_cg4;
extern i16 pad_cg5;
extern i16 pad_cg6;
extern i16 pad_cg7;
extern i16 pad_cg8;
extern i16 pad_cg9;
extern i16 pad_cg10;
extern i16 pad_cg11;
extern i16 pad_cg12;
extern i16 pad_cg13;
extern i16 pad_cg14;
extern i16 pad_cg15;
extern i16 pad_cg16;

/* PORT: mem_set_block_name is in port/ (inline assembly) */
void mem_set_block_name(void *block, char *name);

/* mem_get_name -- 0cc9:0136, 428 bytes */
/* 13 pads: the heapfree-vs-size compares load heapfree (cx:bx) into ax:dx and test the
 * spilled size against it; 0-12 pads give the reverse. */
extern i16 pad_gn0;
extern i16 pad_gn1;
extern i16 pad_gn2;
extern i16 pad_gn3;
extern i16 pad_gn4;
extern i16 pad_gn5;
extern i16 pad_gn6;
extern i16 pad_gn7;
extern i16 pad_gn8;
extern i16 pad_gn9;
extern i16 pad_gn10;
extern i16 pad_gn11;
extern i16 pad_gn12;
/* PORT: mem_get_name is in port/ (inline assembly) */
void *mem_get_name(i32 size, char *tag);

/* mem_get -- 0cc9:02e2, 10 bytes */
void *mem_get(i32 size)
{
    return mem_get_name(size, "$sys$");
}
/* mem_get_block_name -- 0cc9:02ec, 36 bytes */
/* PORT: mem_get_block_name is in port/ (inline assembly) */
void mem_get_block_name(void *block, char *dest);

/* mem_free -- 0cc9:0310, 80 bytes */
/* PORT: mem_free is in port/ (inline assembly) */
i16 mem_free(void *p);

/* mem_adjust -- 0cc9:0360, 46 bytes */
/* PORT: mem_adjust is in port/ (inline assembly) */
i16 mem_adjust(i32 size, void *p);

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
    umb_min_free = min(umb_save_free_area, xms_umb_get_avail());
}

/* The module's uninitialised data (communals, BSS). */
i32 mem_save_free_area;                  /* DS:0x4ae8 */
i32 umb_save_free_area;                  /* DS:0x51c8 */

