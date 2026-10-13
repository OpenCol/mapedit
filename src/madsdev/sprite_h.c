/* From mapedit-decomp src/madsdev/sprite_h (f55aff8), now maintained here: docs/origin.md. */
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


typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { char hdr[4]; i16 count; char rest[0x3c]; IconRec icon[1]; } SpriteSeries;
/* sprite_color_translate -- 1069:0006, 184 bytes */
/* PORT: sprite_color_translate is in port/ (inline assembly) */
void sprite_color_translate(SpriteSeries *sheet, char *map);


