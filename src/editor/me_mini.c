/* Imported from mapedit-decomp src/editor/me_mini by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE me_mini: MAPEDIT.EXE segment 0b56 (me_mini.obj), file 0xcb64-0xd030, 10 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn get_a_color       0x0cb64
 * fn get_a_color_2     0x0cb98
 * fn get_tile_colors   0x0cbcc
 * fn allocate_mini     0x0cc38
 * fn generate_mini     0x0cc3c
 * fn blast_mini_region 0x0cc7c
 * fn blast_mini        0x0cdd4
 * fn show_mini_region  0x0cdee
 * fn show_mini         0x0cf14
 * fn new_mini          0x0d02a
 *
 *  Globals carry their
 * CodeView names (tiles = the icon sheet DS:0x4c0, terrain_1 = the tile sheet, scr_work = the
 * work screen port, main_tile = the backdrop tile port, mini_x/mini_y, map_corner_x..y2).
 *
 * THE 1994 MINIMAP: the DOS side of the Windows MiniWin class (1060:6xxx). The minimap is a
 * 56x39-square window of the map (0x38 x 0x27) drawn one pixel per square straight into the
 * screen port at (0xfc, 9); its top-left square is mini_x/mini_y, which generate_mini keeps
 * around the main map's centre. blast_mini_region is MiniWin_BlastMiniRegion with the GDI rects
 * replaced by a pixel pointer.
 */

// MSC: #pragma optimize("elg", on)
#include "units.h"
#include "terrain.h"
#include "records.h"


extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void *tiles;            /* DS:0x04c0, the icon sheet */
extern void sprite_draw(i16 icon, void *sheet, DosPort *port, i16 x, i16 y);
/* get_a_color -- 0b56:0004, 52 bytes */
/* The colour at the centre (8,8) of a 16x16 icon. */
u8 get_a_color(i16 icon)
{
    u8 buf[256];
    DosPort port;

    port.w = port.h = 16;
    port.bits = buf;
    sprite_draw(icon, tiles, &port, 0, 0);
    return buf[0x88];
}
/* get_a_color_2 -- 0b56:0038, 52 bytes */
extern void *terrain_1;        /* DS:0x04b8, the terrain tile sheet */
extern void tile_draw(void *tiles, i16 tile, DosPort *port, i16 x, i16 y);

/* The colour at the centre (8,8) of a 16x16 terrain tile. */
u8 get_a_color_2(i16 tile)
{
    u8 buf[256];
    DosPort port;

    port.w = port.h = 16;
    port.bits = buf;
    tile_draw(terrain_1, tile, &port, 0, 0);
    return buf[0x88];
}
/* get_tile_colors -- 0b56:006c, 108 bytes */
extern u8 terrain_colors[]; /* DS:0x4aec, 29 */

/* The minimap palette: one colour per base terrain (0-7, repeated for the two forest bands at
 * 8-15 and 16-23), the three water tiles, then mountains and hills from their icons. */
void get_tile_colors(void)
{
    i16 i;

    for (i = 0; i < 8; i++) {
        terrain_colors[i] = get_a_color_2(i);
        terrain_colors[i + 16] = get_a_color_2(i);
        terrain_colors[i + 8] = get_a_color_2(i);
    }
    terrain_colors[24] = get_a_color_2(0x18);
    terrain_colors[25] = get_a_color_2(0x19);
    terrain_colors[26] = get_a_color_2(0x1a);
    terrain_colors[TERRAIN_MOUNTAINS] = get_a_color(0x21);
    terrain_colors[TERRAIN_HILLS] = get_a_color(0x31);
}
/* allocate_mini -- 0b56:00d8, 4 bytes */
i16 allocate_mini(void)
{
    return 0;
}
/* generate_mini -- 0b56:00dc, 64 bytes */
extern i16 map_center_x;           /* DS:0x04c8 */
extern i16 map_center_y;           /* DS:0x04ca */
extern MapSize map_file;           /* DS:0x4b12 */
extern i16 mini_y;                 /* DS:0x49c2 */
extern i16 mini_x;                 /* DS:0x6982 */
extern i16 minimax(i16 v, i16 lo, i16 hi);

void generate_mini(void)
{
    mini_x = map_center_x - 28;
    mini_y = map_center_y - 19;
    mini_x = minimax(mini_x, 1, map_file.w - 57);
    mini_y = minimax(mini_y, 1, map_file.h - 40);
}
/* blast_mini_region -- 0b56:011c, 344 bytes */
extern u8 *map_loc(i16 x, i16 y);
extern u8 *feature_loc(i16 x, i16 y);
extern u8 *site_loc(i16 x, i16 y);
extern u8 *continent_loc(i16 x, i16 y);
extern u8 * buffer_pointer(DosPort *port, i16 x, i16 y);
extern DosPort scr_work;           /* DS:0x3af4 */

extern i16 pad_1, pad_2, pad_3;

/* VICEROY's BlastMiniRegion without the settlement and unit colours (the editor has neither):
 * one pixel per square into scr_work at (0xfc, 9), white when `white`, black where `viewer`
 * has not seen the square, else the terrain colour. The feature and continent planes are still
 * walked into `feature` and `t2`, which nothing reads: under "elg" those reads and the p1/p2
 * steps compile to NOTHING, but they are what gives the shipped frame (0x3a, with their four
 * never-touched words) and the shipped pairing (p0 in si, p3 in di, dst in cx, mask in memory).
 * Without them MSC puts mask in si and the frame is 0x2a (181 bytes differ). */
void blast_mini_region(i16 x, i16 y, i16 cols, i16 rows, i16 viewer, i16 white)
{
    u8 *p0;
    u8 *p1;
    u8 *p3;
    u8 *p2;
    u8 *dst;
    u8 terrain;
    u8 colour;
    u8 feature;
    u8 t2;
    u8 site;
    i16 mapw;
    i16 portw;
    i16 col;
    i16 row;
    i16 mask;

    if (viewer >= 0)
        mask = 0x10 << viewer;
    else
        mask = 0;
    p0 = map_loc(x, y);
    p1 = feature_loc(x, y);
    p3 = site_loc(x, y);
    p2 = continent_loc(x, y);
    dst = buffer_pointer(&scr_work, x - mini_x + 0xfc, y - mini_y + 9);
    mapw = map_file.w;
    portw = scr_work.w;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            terrain = *p0++;
            t2 = *p2++;
            feature = *p1++;
            site = *p3++;
            if (white) {
                colour = 15;
            } else if (mask && !(mask & site)) {
                colour = 0;
            } else {
                if (terrain & MAP0_HILLY) {
                    if (terrain & MAP0_MOUNTAINS)
                        terrain = TERRAIN_MOUNTAINS;
                    else
                        terrain = TERRAIN_HILLS;
                } else {
                    terrain = terrain & MAP0_TERRAIN_MASK;
                }
                colour = terrain_colors[terrain];
            }
            *dst++ = colour;
        }
        p0 += mapw - cols;
        p1 += mapw - cols;
        p2 += mapw - cols;
        p3 += mapw - cols;
        dst += portw - cols;
    }
}
/* blast_mini -- 0b56:0274, 26 bytes */
void blast_mini(i16 viewer)
{
    blast_mini_region(mini_x, mini_y, 0x38, 0x27, viewer, 0);
}
/* show_mini_region -- 0b56:028e, 294 bytes */
#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))
extern i16 map_corner_x;           /* DS:0x49f2 */
extern i16 map_corner_y;           /* DS:0x49f4 */
extern i16 map_corner_x2;          /* DS:0x6026 */
extern i16 map_corner_y2;          /* DS:0x603e */
extern void buffer_draw_box(i16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color);
extern void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

void show_mini_region(i16 x, i16 y, i16 w, i16 h, i16 update, i16 viewer, i16 white)
{
    i16 x0;
    i16 y0;

    generate_mini();
    x0 = max(x, mini_x);
    w = max(min(x + w - 1, mini_x + 55) - x0 + 1, 0);
    y0 = max(y, mini_y);
    h = max(min(y + h - 1, mini_y + 38) - y0 + 1, 0);
    if (w == 0 || h == 0)
        return;
    blast_mini_region(x0, y0, w, h, viewer, white);
    buffer_draw_box(max(mini_x, map_corner_x) - mini_x + 0xfc, max(mini_y, map_corner_y) - mini_y + 9,
               min(mini_x + 55, map_corner_x2) - mini_x + 0xfc, scr_work,
               min(mini_y + 38, map_corner_y2) - mini_y + 9, 15);
    if (update)
        mouse_video_update(x0 - mini_x + 0xfc, y0 - mini_y + 9, x0 - mini_x + 0xfc, y0 - mini_y + 9, w, h);
}
/* show_mini -- 0b56:03b4, 278 bytes */
extern DosPort *main_tile;         /* DS:0x0090 */
extern void buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 color);
extern void buffer_tile(DosPort dst, DosPort src, i16 dx, i16 dy, i16 w, i16 h, i16 sx, i16 sy);

void show_mini(i16 update, i16 viewer)
{
    generate_mini();
    if (main_tile == 0)
        buffer_rect_fill(0xf1, 8, 0x4f, scr_work, 0x29, 0);
    else
        buffer_tile(scr_work, *main_tile, 0xf1, 8, 0x4f, 0x29, 0, 0);
    buffer_draw_box(0xfb, 8, 0x134, scr_work, 0x30, 6);
    blast_mini(viewer);
    buffer_draw_box(max(mini_x, map_corner_x) - mini_x + 0xfc, max(mini_y, map_corner_y) - mini_y + 9,
               min(mini_x + 55, map_corner_x2) - mini_x + 0xfc, scr_work,
               min(mini_y + 38, map_corner_y2) - mini_y + 9, 15);
    if (update)
        mouse_video_update(0xf1, 8, 0xf1, 8, 0x4f, 0x29);
}
/* new_mini -- 0b56:04ca, 6 bytes */
void new_mini(void)
{
    generate_mini();
}

