/* From mapedit-decomp src/madsdev/buffer_a (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_a: MAPEDIT.EXE segment 0c80 (buffer_a.c), file 0xde00-0xde6c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_vline 0x0de00
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bc3).
 *
 * A VERTICAL LINE (root code, frame 0bc3). buffer_vline(x, y0, y1, port, color) (_fastcall) clips
 * to the port in C, then draws in inline assembly.
 */

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

/* buffer_vline -- 0c80:0000, 108 bytes */
/* PORT: buffer_vline is in port/ (inline assembly) */
void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color);


