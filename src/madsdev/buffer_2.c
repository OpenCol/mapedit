/* Imported from mapedit-decomp src/madsdev/buffer_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_2: MAPEDIT.EXE segment 0c46 (BUFFER_2.C), file 0xda66-0xda9e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_free  0x0da66
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl31_7881e).
 *
 * FREEING A DRAWING PORT (overlay 31, para 29). DOS-only engine code; _fastcall, with the
 * port in bx. It returns -1
 * when it freed something, otherwise 0.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern i16 mem_free(void *p);
/* buffer_free -- 0c46:0006, 56 bytes */
i16 buffer_free(DosPort *port)
{
    i16 freed;

    freed = 0;
    if (port->bits != 0) {
        mem_free(port->bits);
        freed = -1;
    }
    port->bits = 0;
    port->w = 0;
    port->h = 0;
    return freed;
}

