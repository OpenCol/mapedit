/* Imported from mapedit-decomp src/madsdev/sprite_m by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sprite_m: MAPEDIT.EXE segment 0e92 (sprite_m.c), file 0xff2a-0x1020a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_draw_scaled_black 0x0ff2a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0caa).
 *
 * DRAWING A SCALED ICON UNDERNEATH (root code, frame 0caa). sprite_draw_scaled_black(icon,
 * sheet, port, x, y, scale) (_fastcall) is draw_icon_scaled (root_0c56) except that it only
 * paints pixels that are still 0 (draw_map_icon2 below full scale). Everything after the
 * port bounds is inline assembly.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[0x36]; IconRec icon[1]; } IconSheet;
/* sprite_draw_scaled_black -- 0e92:000a, 736 bytes */
/* PORT: sprite_draw_scaled_black is in port/ (inline assembly) */
void sprite_draw_scaled_black(i16 icon, IconSheet *sheet, DosPort *port, i16 x, i16 y, i16 scale);


