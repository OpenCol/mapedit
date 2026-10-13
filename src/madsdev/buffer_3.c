/* Imported from mapedit-decomp src/madsdev/buffer_3 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern void buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);
/* buffer_fill -- 0c49:000e, 34 bytes */
void buffer_fill(char c, DosPort port)
{
    buffer_rect_fill(0, 0, port.w, port, port.h, c);
}

