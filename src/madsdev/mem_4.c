/* Imported from mapedit-decomp src/madsdev/mem_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i32 xms_umb_get_avail(void);                /* 127e:000c */
i32 mem_conv_get_avail(void);
/* mem_conv_largest -- 0d08:000e, 90 bytes */
/* PORT: mem_conv_largest is in port/ (inline assembly) */
u16 mem_conv_largest(void);

/* mem_conv_get_avail -- 0d08:0068, 26 bytes */
/* PORT: mem_conv_get_avail is in port/ (inline assembly) */
i32 mem_conv_get_avail(void);

/* mem_get_avail -- 0d08:0082, 40 bytes */
i32 mem_get_avail(void)
{
    i32 dos;
    i32 heap;

    dos = mem_conv_get_avail();
    heap = xms_umb_get_avail();
    return heap >= dos ? heap : dos;
}
/* mem_set_video_mode -- 0d08:00aa, 2 bytes */
void mem_set_video_mode(void)
{
}

