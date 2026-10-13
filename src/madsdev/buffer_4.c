/* From mapedit-decomp src/madsdev/buffer_4 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_4: MAPEDIT.EXE segment 0c4c (buffer_4.c), file 0xdac0-0xdbb4, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_copy 0x0dac0
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b8f).
 *
 * COPYING A RECTANGLE BETWEEN PORTS (root code, frame 0b8f). buffer_rect_copy(x, y, w, src, dst, h)
 * (_fastcall) copies (x, y, w, h) from src to the same place in dst when both have pixels
 * and returns whether it did. The rows are copied in inline assembly: word moves, an odd
 * byte, and a 32 KB segment step on each side.
 */



extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
extern char *mem_normalize(char *p);
/* buffer_rect_copy -- 0c4c:0000, 244 bytes */
/* PORT: buffer_rect_copy is in port/ (inline assembly) */
i16 buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h);


