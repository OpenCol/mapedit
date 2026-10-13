/* Imported from mapedit-decomp src/madsdev/buffer_b by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_b: MAPEDIT.EXE segment 0c86 (buffer_b.c), file 0xde6c-0xdf10, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_draw_box 0x0de6c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bca).
 *
 * A RECTANGLE OUTLINE (root code, frame 0bca). buffer_draw_box(x0, y0, x1, port, y1, color)
 * (_fastcall) orders the corners, then draws two hlines and two vlines.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; void *bits; } DosPort;

extern void buffer_hline(i16 x0, i16 x1, i16 y, DosPort port, char color);
extern void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color);
/* buffer_draw_box -- 0c86:000c, 164 bytes */
void buffer_draw_box(i16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color)
{
    i16 t;

    if (x1 < x0) {
        t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 < y0) {
        t = y0;
        y0 = y1;
        y1 = t;
    }
    buffer_hline(x0, x1, y0, port, color);
    buffer_hline(x0, x1, y1, port, color);
    buffer_vline(x0, y0, y1, port, color);
    buffer_vline(x1, y0, y1, port, color);
}

