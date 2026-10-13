/* Imported from mapedit-decomp src/madsdev/sprite_h by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE sprite_h: MAPEDIT.EXE segment 1069 (SPRITE_H.C), file 0x11c96-0x11d4e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_color_translate 0x11c96
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * RECOLOURING A SPRITE SERIES IN PLACE. sprite_color_translate(sheet, map) (_fastcall)
 * walks every frame of a loaded sprite series (frame data pointers at 0x42 + 12 * i, the
 * frame count at +4) and rewrites each pixel byte of the run-length data through the
 * translation map: entry `map + 5 + 6 * colour`. The decoder (inline assembly) follows the
 * sprite format: 0xfc ends a frame, 0xff ends a line, 0xfd opens a run-mode line (pairs of
 * count and colour, 0xfd keeping transparency), and in plain mode 0xfe starts a repeat
 * (two bytes skipped before the colour). A frame with no data stops the walk.
 * Locals named for the frame slots (data5 at bp-4, tab at bp-8, i at bp-0xa).
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[4]; i16 count; char rest[0x3c]; IconRec icon[1]; } SpriteSeries;
/* sprite_color_translate -- 1069:0006, 184 bytes */
/* PORT: sprite_color_translate is in port/ (inline assembly) */
void sprite_color_translate(SpriteSeries *sheet, char *map);


