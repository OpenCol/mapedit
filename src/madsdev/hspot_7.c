/* From mapedit-decomp src/madsdev/hspot_7 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE hspot_7: MAPEDIT.EXE segment 0baa (HSPOT_7.C), file 0xd0a2-0xd0f4, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn hspot_which  0x0d0a2
 *
 * No VICEROY twin: reconstructed from the shipped bytes. The body is inline assembly (the
 * prologue homes all three register parameters); the nop before `none:` is MSC's own
 * padding of a label that follows an unconditional jmp to an even offset.
 *
 * hspot_which(x, y, id) (_fastcall) returns the index (from 1) of the first active hot
 * spot with that id containing (x, y), or 0. The coordinates compare unsigned.
 */



extern i16 numspots;                             /* DS:0x0716 (hspot_1.c) */
extern HotSpot spot[];                           /* DS:0x530a */
/* hspot_which -- 0baa:0002, 82 bytes */
/* PORT: hspot_which is in port/ (inline assembly) */
i16 hspot_which(u16 x, u16 y, i16 id);


