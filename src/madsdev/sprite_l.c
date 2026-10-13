/* Imported from mapedit-decomp src/madsdev/sprite_l by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sprite_l: MAPEDIT.EXE segment 0e71 (sprite_l.c), file 0xfd1c-0xff2a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_draw_black 0x0fd1c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c89).
 *
 * DRAWING AN ICON UNDERNEATH (root code, frame 0c89). sprite_draw_black(icon, sheet, port, x,
 * y) (_fastcall) is draw_icon (root_0c36) except that it only paints pixels that are still
 * 0, so the icon shows behind what the port already holds (draw_map_icon2 at full scale).
 * Same icon records, RLE codes and mirroring; everything after the port bounds is inline
 * assembly.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[0x36]; IconRec icon[1]; } IconSheet;
/* sprite_draw_black -- 0e71:000c, 526 bytes */
/* PORT: sprite_draw_black is in port/ (inline assembly) */
void sprite_draw_black(i16 icon, IconSheet *sheet, DosPort *port, i16 x, i16 y);


