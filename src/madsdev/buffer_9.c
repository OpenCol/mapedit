/* From mapedit-decomp src/madsdev/buffer_9 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_9: MAPEDIT.EXE segment 0c79 (buffer_9.c), file 0xdd96-0xde00, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_hline 0x0dd96
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bbc).
 *
 * A HORIZONTAL LINE (root code, frame 0bbc). buffer_hline(x0, x1, y, port, color) (_fastcall) clips
 * to the port in C, then draws in inline assembly. Compare vline (root_0bc3.c), whose
 * loop does not add the port's offset.
 */

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

/* buffer_hline -- 0c79:0006, 106 bytes */
/* PORT: buffer_hline is in port/ (inline assembly) */
void buffer_hline(i16 x0, i16 x1, i16 y, DosPort port, char color);


