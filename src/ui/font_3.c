/* From mapedit-decomp src/madsdev/font_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "popup.h"
#include "font.h"
#include "loader.h"
#include "records.h"
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


/* font_set_colors -- 0d6a:0006, 28 bytes */
void font_set_colors(i16 a, i16 b, i16 c, i16 d)
{
    font_colors[0] = a;
    font_colors[1] = b;
    font_colors[2] = c;
    font_colors[3] = d;
}

