/* From mapedit-decomp src/editor/map_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "error.h"
#include "pack.h"
#include "timer.h"
#include "font.h"
#include "menu.h"
#include "popup.h"
#include "sprite.h"
#include "buffer.h"
#include "loader.h"
#include "map.h"
#include "mouse.h"
#include "screen.h"
/* MODULE map_2: MAPEDIT.EXE segment 087b (map_2.obj), file 0x9db4-0xa02a, 7 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn conform_to_view       0x09db4
 * fn conform_to_view_size  0x09dfa
 * fn update_terrain_region 0x09e70
 * fn update_terrain        0x09eda
 * fn show_map_gradually    0x09f10
 * fn show_map              0x09fcc
 * fn show_map_region       0x09fe4
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl21_66e0c).
 *
 * THE 1994 MAP-VIEW PLUMBING: Windows conform_to_view_size, update_terrain_region, show_map_region
 * (1040:xxxx) and the whole-map update_terrain / show_map, with the MapWin
 * and MiniWin calls replaced by blits. The map is drawn into the backdrop port (DS:0x839e) and
 * copied to the screen port at y + 8 (below the menu bar).
 */

/* plain flags */
#include "records.h"

/* conform_to_view -- 087b:0004, 70 bytes */
/* Clips a corner pair to the visible map squares. */
void conform_to_view(i16 *x0, i16 *y0, i16 *x1, i16 *y1)
{
    *x0 = max(*x0, map_corner_x);
    *y0 = max(*y0, map_corner_y);
    *x1 = min(*x1, map_corner_x2);
    *y1 = min(*y1, map_corner_y2);
}
/* conform_to_view_size -- 087b:004a, 118 bytes */
void conform_to_view_size(i16 *x, i16 *y, i16 *w, i16 *h)
{
    i16 x1;
    i16 y1;

    x1 = *x + *w - 1;
    y1 = *y + *h - 1;
    *x = max(*x, map_corner_x);
    *y = max(*y, map_corner_y);
    x1 = min(x1, map_corner_x2);
    y1 = min(y1, map_corner_y2);
    *w = x1 - *x + 1;
    *h = y1 - *y + 1;
    *w = max(0, *w);
    *h = max(0, *h);
}
/* update_terrain_region -- 087b:00c0, 106 bytes */

void update_terrain_region(i16 x, i16 y, i16 w, i16 h)
{
    i16 px;
    i16 py;
    i16 pw;
    i16 ph;

    px = (x - map_corner_x + map_tile_inset_x) * map_tile_size_x;
    py = (y - map_corner_y + map_tile_inset_y) * map_tile_size_y;
    pw = map_tile_size_x * w;
    ph = map_tile_size_y * h;
    buffer_rect_copy_2(px, py, px, scr_orig, scr_work, py + 8, pw, ph);
}
/* update_terrain -- 087b:012a, 54 bytes */

void update_terrain(void)
{
    buffer_rect_copy_2(0, 0, 0, scr_orig, scr_work, 8, map_pixel_size_x, map_pixel_size_y);
}
/* show_map_gradually -- 087b:0160, 188 bytes */

/* Reveals the map view square by square in a pseudo-random order (a 16-bit LFSR). */
/* PORT: show_map_gradually is in port/ (inline assembly) */

/* show_map -- 087b:021c, 24 bytes */
void show_map(void)
{
    mouse_video_update(0, 8, 0, 8, map_pixel_size_x, map_pixel_size_y);
}
/* show_map_region -- 087b:0234, 70 bytes */
void show_map_region(i16 x, i16 y, i16 w, i16 h)
{
    i16 px;
    i16 py;
    i16 pw;
    i16 ph;

    px = (x - map_corner_x + map_tile_inset_x) * map_tile_size_x;
    py = (y - map_corner_y + map_tile_inset_y) * map_tile_size_y + 8;
    pw = map_tile_size_x * w;
    ph = map_tile_size_y * h;
    mouse_video_update(px, py, px, py, pw, ph);
}

/* show_map_gradually -- 087b:0160: copy the map view to the screen a square at a time, in the
   order of a 16-bit Galois LFSR (taps B400h) over 65,536 steps; a small view (180 squares or
   fewer) waits a 600 Hz tick between squares */
void show_map_gradually(void)
{
    u16 lfsr = 1, total, pos, count = 0;
    i32 oldt = 0;
    total = (u16)(map_view_size_y * map_view_size_x);
    timer_600_low = 0;
    for (;;) {
        lfsr = (u16)((lfsr & 1) ? (lfsr >> 1) ^ 0xb400 : lfsr >> 1);
        pos = (u16)(lfsr - 1);
        if (pos < total) {
            i16 col = (i16)(pos % (u16)map_view_size_x), row = (i16)(pos / (u16)map_view_size_x);
            i16 sx = (i16)((col + map_tile_inset_x) * map_tile_size_x);
            i16 sy = (i16)((row + map_tile_inset_y) * map_tile_size_y + 8);
            mouse_video_update(sx, sy, sx, sy, map_tile_size_x, map_tile_size_y);
            if (total <= 0xb4) {
                if (oldt != 0)
                    while (timer_read_600() - oldt < 1)
                        ;
                oldt = timer_read_600();
            }
        }
        if (++count == 0)
            break;
    }
}
