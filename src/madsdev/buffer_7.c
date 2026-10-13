/* From mapedit-decomp src/madsdev/buffer_7 (f55aff8), now maintained here: docs/origin.md. */
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



extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
/* buffer_put -- 0c76:0004, 28 bytes */
void buffer_put(i16 x, i16 y, i16 v, DosPort port)
{
    *buffer_pointer(&port, x, y) = (char)v;
}

