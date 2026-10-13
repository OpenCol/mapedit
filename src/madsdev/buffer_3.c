/* From mapedit-decomp src/madsdev/buffer_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_3: MAPEDIT.EXE segment 0c49 (BUFFER_3.C), file 0xda9e-0xdac0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_fill  0x0da9e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b8d).
 *
 * FILLING A WHOLE PORT (root code, frame 0b8d). buffer_fill(c, port) (_fastcall, the
 * colour in al) fills the port with c.
 */



extern void buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);
/* buffer_fill -- 0c49:000e, 34 bytes */
void buffer_fill(char c, DosPort port)
{
    buffer_rect_fill(0, 0, port.w, port, port.h, c);
}

