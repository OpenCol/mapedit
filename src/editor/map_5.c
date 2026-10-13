/* Imported from mapedit-decomp src/editor/map_5 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE map_5: MAPEDIT.EXE segment 08a2 (map_5.obj), file 0xa02a-0xa0d0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn draw_map_region_2 0x0a02a
 * fn draw_map_2        0x0a088
 *
 * No VICEROY twin: reconstructed from the shipped bytes. The editor's whole-map redraw:
 * draw_map_region_2(x, y, w, h) clamps the region to the view (conform_to_view_size), renders
 * its terrain for every nation (-1), copies it to the screen and refreshes the minimap;
 * draw_map_2 clears the work screen (buffer_draw_box over scr_work, colour 0) and does the same
 * for the whole view.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern void conform_to_view_size(i16 *x, i16 *y, i16 *w, i16 *h);
extern void generate_terrain_map_region(i16 x, i16 y, i16 w, i16 h, i16 nation);
extern void update_terrain_region(i16 x, i16 y, i16 w, i16 h);
extern void show_map_region(i16 x, i16 y, i16 w, i16 h);
extern void show_mini(i16 update, i16 viewer);
/* draw_map_region_2 -- 08a2:000a, 94 bytes */
void draw_map_region_2(i16 x, i16 y, i16 w, i16 h)
{
    conform_to_view_size(&x, &y, &w, &h);
    generate_terrain_map_region(x, y, w, h, -1);
    update_terrain_region(x, y, w, h);
    show_map_region(x, y, w, h);
    show_mini(1, -1);
}
/* draw_map_2 -- 08a2:0068, 72 bytes */
extern DosPort scr_work;                    /* DS:0x3af4 */
extern i16 map_pixel_size_x;                /* DS:0x64ec */
extern i16 map_pixel_size_y;                /* DS:0x64ee */
extern void buffer_draw_box(u16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color);
extern void generate_terrain_map(i16 nation);
extern void update_terrain(void);
extern void show_map(void);

void draw_map_2(void)
{
    buffer_draw_box(-1, 7, map_pixel_size_x, scr_work, map_pixel_size_y + 8, 0);
    generate_terrain_map(-1);
    update_terrain();
    show_map();
    show_mini(1, -1);
}

