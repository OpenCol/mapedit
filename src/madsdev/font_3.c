/* Imported from mapedit-decomp src/madsdev/font_3 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE font_3: MAPEDIT.EXE segment 0d6a (FONT_3.C), file 0xeca6-0xecc2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_set_colors 0x0eca6
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c28).
 *
 * TEXT COLOURS (root code, frame 0c28; "elg"). font_set_colors(a, b, c, d) (_fastcall) stores the four
 * colour bytes draw_text reads (DS:0x269e-0x26a1); callers pass -1 for "unchanged" in a.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern u8 font_colors[4];    /* DS:0x269e */
/* font_set_colors -- 0d6a:0006, 28 bytes */
// MSC: #pragma optimize("elg", on)
void font_set_colors(i16 a, i16 b, i16 c, i16 d)
{
    font_colors[0] = a;
    font_colors[1] = b;
    font_colors[2] = c;
    font_colors[3] = d;
}

