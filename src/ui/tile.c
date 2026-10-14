/* tile.c -- 16 x 16 terrain tiles: allocating a sheet and drawing tiles, plain, masked and
 * scaled.
 *
 * A tile sheet is n tiles of 256 bytes, each 16 rows of 16 palette indices; tile t starts at
 * t << 8. The drawers take a terrain number, which tile_id turns into a sheet slot, and draw
 * into a DosPort without clipping. The _mask variants treat a destination pixel that is not 0
 * as already covered and leave it; the _scale variants draw the tile shrunk by a power of two,
 * for the map's zoomed-out views.
 */
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
#pragma pack(2)   /* records packed on 2-byte boundaries, as in 1994; file formats rely on it */

/* tile_allocate -- allocate a sheet of n tiles (n * 256 bytes); 0 when memory runs out */
char *tile_allocate(i16 n)
{
    char *p;
    i32 size;

    return mem_get((i32)n << 8);
}
/* tile_id -- the sheet slot of terrain number t: 9 and 0x11 share slot 8, numbers below 8 are
 * their own slot, and the rest move down by 15. Nothing guards the numbers that would map
 * below 0 (8, 10-14).
 */
i16 tile_id(i16 t)
{
    if (t == 0x11 || t == 9)
        t = 8;
    else if (t >= 8)
        t -= 0x0f;
    return t;
}
/* tile_draw -- draw the tile for terrain t with its top left corner at (x, y), all 256 pixels */
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

/* tile_draw_mask -- tile_draw, writing only where the destination pixel is 0 */
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

/* tile_draw_scale -- draw the tile for terrain t shrunk by 1 << shift: (16 >> shift) pixels
 * square, each taken from the centre of its step x step block of the tile (step/2 across and
 * down). The result is centred on x, and its bottom row is on y.
 */
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

/* tile_draw_scale_mask -- tile_draw_scale, writing only over pixels that are 0.
 * Quirk of the original, kept: at a destination pixel that is not 0 it advances the source by
 * one byte more than a step and does not advance the destination, so the next source pixel is
 * tested against the same, still covered, byte. The first covered pixel therefore ends the
 * row's drawing, and because the row skip is fixed at port->w - n, the next row starts as many
 * pixels left of its column as the row had pixels left to draw.
 */
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
