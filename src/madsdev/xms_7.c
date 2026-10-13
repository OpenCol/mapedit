/* From mapedit-decomp src/madsdev/xms_7 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_7: MAPEDIT.EXE segment 1343 (XMS_7.C), file 0x14a34-0x14a5a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_copy     0x14a34
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * AN EXTENDED-MEMORY MOVE (madsdev.lib). xms_copy's five arguments are the XMS move
 * structure (function 0Bh) laid out on the stack: byte count, source handle and offset,
 * destination handle and offset -- a handle of 0 makes the offset a far pointer into
 * conventional memory. ds:si points at the structure (the `mov si,6; add si,bp` is the
 * programmer's own asm). 0 on success, -1 with no driver or a failed move.
 */

extern i16 xms_exists;                      /* DS:0x3c68 (xms_1.c) */
extern void (*xms_controller)(void);    /* DS:0x3c6c (xms_1.c) */
/* xms_copy -- 1343:0004, 38 bytes */
/* PORT: xms_copy is in port/ (inline assembly) */
i16 xms_copy(i32 length, i16 src_handle, i32 src_offset, i16 dst_handle, i32 dst_offset);


