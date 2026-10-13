/* Imported from mapedit-decomp src/madsdev/buffer_8 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_8: MAPEDIT.EXE segment 0c78 (buffer_8.c), file 0xdd80-0xdd96, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_get   0x0dd80
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bbb).
 *
 * READING ONE BYTE FROM A PORT (root code, frame 0bbb). buffer_get(x, y, port) (_fastcall).
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
/* buffer_get -- 0c78:0000, 22 bytes */
u8 buffer_get(i16 x, i16 y, DosPort port)
{
    return *buffer_pointer(&port, x, y);
}

