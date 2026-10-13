/* tile.c -- the 16 x 16 terrain tile drawers of tile.obj, whose loops are inline assembly in
 * the original. A tile sheet is n tiles of 256 bytes; tile t starts at t << 8. A destination
 * pixel that is not 0 is "under" something already drawn: the _mask variants leave it.
 */
#include "msc.h"
#include "buffer.h"
#include "map.h"
#include "records.h"
#include "units.h"
#include "port_records.h"


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
