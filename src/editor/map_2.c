/* Imported from mapedit-decomp src/editor/map_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

typedef struct { i16 h; i16 w; void *bits; } DosPort;
#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i16 map_corner_x;                         /* DS:0x8328 */
extern i16 map_corner_y;                         /* DS:0x832e */
extern i16 map_corner_x2;                         /* DS:0x8804 */
extern i16 pad_1, pad_2, pad_3, pad_4, pad_5, pad_6, pad_7;
extern i16 map_corner_y2;                         /* DS:0x8806 */
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
extern i16 pad_8, pad_9, pad_10, pad_11, pad_12, pad_13, pad_14, pad_15;
extern i16 pad_16, pad_17, pad_18, pad_19, pad_20, pad_21, pad_22;
extern i16 map_tile_size_x;                          /* DS:0x5ad4 */
extern i16 map_tile_size_y;                          /* DS:0x8326 */
extern i16 pad_23, pad_24, pad_25, pad_26, pad_27, pad_28, pad_29, pad_30;
extern i16 pad_31, pad_32, pad_33, pad_34, pad_35, pad_36, pad_37, pad_38;
extern i16 map_tile_inset_x;                       /* DS:0x832a, screen square of map_corner_x */
extern i16 map_tile_inset_y;                       /* DS:0x832c */
extern i16 pad_39, pad_40, pad_41, pad_42, pad_43, pad_44, pad_45;
extern DosPort scr_orig;                 /* DS:0x839e, the backdrop port */
extern DosPort scr_work;                 /* DS:0x2da8 */
extern void buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h);

extern i16 pad_46, pad_47, pad_48, pad_49, pad_50, pad_51, pad_52, pad_53;
extern i16 pad_54, pad_55, pad_56, pad_57, pad_58;
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
extern i16 map_pixel_size_x;                         /* DS:0x8550 */
extern i16 map_pixel_size_y;                         /* DS:0x8552 */

void update_terrain(void)
{
    buffer_rect_copy_2(0, 0, 0, scr_orig, scr_work, 8, map_pixel_size_x, map_pixel_size_y);
}
/* show_map_gradually -- 087b:0160, 188 bytes */
extern i16 map_view_size_y;                       /* DS:0x8546 */
extern i16 map_view_size_x;                       /* DS:0x8544 */
extern i32 timer_600_low;                      /* DS:0x8338 */
extern i32 timer_read_600(void);
extern void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

/* Reveals the map view square by square in a pseudo-random order (a 16-bit LFSR). */
/* PORT: show_map_gradually is in port/ (inline assembly) */
void show_map_gradually(void);

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

