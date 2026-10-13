/* Imported from mapedit-decomp src/madsdev/mem_3 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* mem_check_overflow -- 1357:000e, 24 bytes */
/* PORT: mem_check_overflow is in port/ (inline assembly) */
void *mem_check_overflow(void *p);


