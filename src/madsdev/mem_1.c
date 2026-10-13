/* Imported from mapedit-decomp src/madsdev/mem_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* mem_normalize -- 104f:0002, 24 bytes */
/* PORT: mem_normalize is in port/ (inline assembly) */
char * mem_normalize(char *p);


