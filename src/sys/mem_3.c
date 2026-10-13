/* From mapedit-decomp src/madsdev/mem_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
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


