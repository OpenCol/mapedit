/* From mapedit-decomp src/editor/tile (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "map.h"
#include "mem.h"
#include "records.h"
#include "units.h"
/* MODULE tile: MAPEDIT.EXE segment 030b (tile.obj), file 0x46b6-0x494a, 6 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn tile_allocate        0x046b6
 * fn tile_id              0x046ce
 * fn tile_draw            0x046f8
 * fn tile_draw_mask       0x0475c
 * fn tile_draw_scale      0x047ce
 * fn tile_draw_scale_mask 0x04884
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0101).
 *
 * 16x16 TILES (root code, frame 0101). A tile sheet is n 256-byte tiles.
 *   - tile_allocate(n) allocates one (mem_get).
 *   - tile_id(t) maps terrain numbers to sheet slots: 0x11 and 9 share slot 8, and
 *     anything from 8 up moves down by 15.
 *   - tile_draw(tiles, t, port, x, y) copies tile t into the port at (x, y);
 *     tile_draw_mask does the same but only into zero (transparent) destination pixels.
 *     Both copy in inline assembly.
 * The blitters' local names (dst, skip5, w6, src6) are chosen for the slot buckets.
 *   - tile_draw_scale / 0286(tiles, t, port, x, y, shift) draw the tile shrunk by 1 << shift,
 *     centred on x and bottom-aligned on y; 0286 is the transparent one. Their locals are
 *     also named for the slot buckets.
 */



/* tile_allocate -- 030b:0006, 24 bytes */
char *tile_allocate(i16 n)
{
    char *p;
    i32 size;

    return mem_get((i32)n << 8);
}
/* tile_id -- 030b:001e, 42 bytes */
i16 tile_id(i16 t)
{
    if (t == 0x11 || t == 9)
        t = 8;
    else if (t >= 8)
        t -= 0x0f;
    return t;
}
/* tile_draw -- 030b:0048, 100 bytes */
/* PORT: tile_draw is in port/ (inline assembly) */

/* tile_draw_mask -- 030b:00ac, 114 bytes */
/* PORT: tile_draw_mask is in port/ (inline assembly) */

/* tile_draw_scale -- 030b:011e, 182 bytes */
/* PORT: tile_draw_scale is in port/ (inline assembly) */

/* tile_draw_scale_mask -- 030b:01d4, 198 bytes */
/* PORT: tile_draw_scale_mask is in port/ (inline assembly) */


