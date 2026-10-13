/* From mapedit-decomp src/madsdev/mem_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
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


