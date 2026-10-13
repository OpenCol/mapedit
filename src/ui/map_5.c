/* From mapedit-decomp src/editor/map_5 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "popup.h"
#include "buffer.h"
#include "loader.h"
#include "map.h"
#include "records.h"
#include "screen.h"
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

void draw_map_2(void)
{
    buffer_draw_box(-1, 7, map_pixel_size_x, scr_work, map_pixel_size_y + 8, 0);
    generate_terrain_map(-1);
    update_terrain();
    show_map();
    show_mini(1, -1);
}

