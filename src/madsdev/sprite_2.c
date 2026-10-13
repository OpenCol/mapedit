/* From mapedit-decomp src/madsdev/sprite_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sprite_2: MAPEDIT.EXE segment 0dae (sprite_2.c), file 0xf0ea-0xf3b8, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_draw_scaled 0x0f0ea
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c56).
 *
 * DRAWING A SCALED ICON (root code, frame 0c56). sprite_draw_scaled(icon, sheet, port, x, y,
 * scale) (_fastcall) is draw_icon shrunk to scale percent: a table marks which source
 * rows and columns survive (an accumulator from 50 that keeps a line each time it passes
 * 100), the icon is centred on x and bottom-aligned on y, and the decoder skips the
 * dropped rows and columns. Everything after the port bounds is inline assembly.
 */


typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[0x36]; IconRec icon[1]; } IconSheet;
/* sprite_draw_scaled -- 0dae:000a, 718 bytes */
/* PORT: sprite_draw_scaled is in port/ (inline assembly) */
void sprite_draw_scaled(i16 icon, IconSheet *sheet, DosPort *port, i16 x, i16 y, i16 scale);


