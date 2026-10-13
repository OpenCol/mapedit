/* Imported from mapedit-decomp src/madsdev/buffer_7 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_7: MAPEDIT.EXE segment 0c76 (buffer_7.c), file 0xdd64-0xdd80, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_put   0x0dd64
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bb9).
 *
 * PUTTING ONE BYTE INTO A PORT (root code, frame 0bb9). buffer_put(x, y, v, port) (_fastcall).
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
/* buffer_put -- 0c76:0004, 28 bytes */
void buffer_put(i16 x, i16 y, i16 v, DosPort port)
{
    *buffer_pointer(&port, x, y) = (char)v;
}

