/* tile.c -- tiles: allocating them and drawing them, plain, masked and scaled. */
#include "msc.h"
#include "buffer.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mem.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "port_records.h"
#include "records.h"
#include "units.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

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

/* tile.c -- the 16 x 16 terrain tile drawers of tile.obj, whose loops are inline assembly in
 * the original. A tile sheet is n tiles of 256 bytes; tile t starts at t << 8. A destination
 * pixel that is not 0 is "under" something already drawn: the _mask variants leave it.
 */


/* tile_draw -- 030b:0048: copy tile t to (x, y), all 256 pixels */
void tile_draw(char *tiles, i16 t, DosPort *port, i16 x, i16 y)
{
    char *d = buffer_pointer(port, x, y);
    const char *s;
    i16 r, c;
    t = tile_id(t);
    s = tiles + (u16)(t << 8);
    for (r = 0; r < 16; r++, d += port->w)
        for (c = 0; c < 16; c++)
            d[c] = *s++;
}

/* tile_draw_mask -- 030b:00ac: the same, writing only where the destination pixel is 0 */
void tile_draw_mask(char *tiles, i16 t, DosPort *port, i16 x, i16 y)
{
    char *d = buffer_pointer(port, x, y);
    const char *s;
    i16 r, c;
    t = tile_id(t);
    s = tiles + (u16)(t << 8);
    for (r = 0; r < 16; r++, d += port->w)
        for (c = 0; c < 16; c++, s++)
            if (d[c] == 0)
                d[c] = *s;
}

/* tile_draw_scale -- 030b:011e: the tile shrunk by 1 << shift (one pixel of each
   step x step block, the one at its centre: +step/2 across and down), drawn centred on x
   and bottom-aligned on y */
void tile_draw_scale(char *tiles, i16 t, DosPort *port, i16 x, i16 y, i16 shift)
{
    i16 n = (i16)(16 >> shift), step = (i16)(1 << shift), half = (i16)(step >> 1), r, c;
    const char *row;
    char *d;
    t = tile_id(t);
    y -= n - 1;
    x -= n >> 1;
    d = buffer_pointer(port, x, y);
    row = tiles + (u16)(t << 8) + half + (half << 4);
    for (r = 0; r < n; r++, row += 16 << shift) {
        const char *s = row;
        for (c = 0; c < n; c++, s += step)
            *d++ = *s;
        d += port->w - n;
    }
}

/* tile_draw_scale_mask -- 030b:01d4: tile_draw_scale writing only over pixels that are 0.
   Where a destination pixel is not 0 the original skips one more source byte and does NOT
   move on in the destination (INC SI; ADD SI,step -- no INC DI), so the next pixel of the
   row lands on the same byte. That is the shipped editor's behaviour, and the port's. */
void tile_draw_scale_mask(char *tiles, i16 t, DosPort *port, i16 x, i16 y, i16 shift)
{
    i16 n = (i16)(16 >> shift), step = (i16)(1 << shift), half = (i16)(step >> 1), r, c;
    const char *row;
    char *d;
    t = tile_id(t);
    y -= n - 1;
    x -= n >> 1;
    d = buffer_pointer(port, x, y);
    row = tiles + (u16)(t << 8) + half + (half << 4);
    for (r = 0; r < n; r++, row += 16 << shift) {
        const char *s = row;
        for (c = 0; c < n; c++) {
            if (*d == 0) {
                *d++ = *s;
                s += step;
            } else
                s += 1 + step;
        }
        d += port->w - n;
    }
}
