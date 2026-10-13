/* From mapedit-decomp src/madsdev/sprite_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sprite_1: MAPEDIT.EXE segment 0d8f (sprite_1.c), file 0xeef0-0xf0ea, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_draw  0x0eef0
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c36).
 *
 * DRAWING AN ICON (root code, frame 0c36). sprite_draw(icon, sheet, port, x, y) (_fastcall)
 * draws entry icon of the sheet at (x, y), mirrored left-right when icon is negative. The
 * entries are 12-byte records from sheet+0x36 (RLE data far pointer, width at +8, height
 * at +0xa). Everything after the port bounds is inline assembly; per row, 0xff ends the icon,
 * a leading 0xfd marks a row of (count, colour) runs, otherwise plain bytes with 0xfe
 * (count, colour) runs; colour 0xfd is transparent.
 */


typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[0x36]; IconRec icon[1]; } IconSheet;
/* sprite_draw -- 0d8f:0000, 506 bytes */
/* PORT: sprite_draw is in port/ (inline assembly) */
void sprite_draw(i16 icon, IconSheet *sheet, DosPort *port, i16 x, i16 y);


