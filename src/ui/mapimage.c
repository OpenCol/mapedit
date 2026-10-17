/* mapimage.c -- Export Map as PNG (port: options.h, export_png).
 *
 * The whole map as the view draws it at zoom 0, 16 pixels a square, without the border ring:
 * a 58 x 72 map gives 896 x 1120 pixels. The view's renderer draws into scr_orig, one view
 * (15 x 12 squares) at a time, so the map is drawn view by view, each copied into the picture;
 * the views at the far edges overlap the ones before them, since the view stays inside the
 * border. Coasts, seams, rivers and roads look at the neighbours in the planes, not in the
 * view, so the views join without seams. The colours are the palette the screen shows.
 */
#include <stdlib.h>
#include <string.h>

#include "msc.h"
#include "map.h"
#include "mapimage.h"
#include "mapview.h"
#include "options.h"
#include "png.h"
#include "records.h"
#include "screen.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */

i16 export_map_png(char *where, i16 cap)
{
    i16 scale = map_scale, cx = map_center_x, cy = map_center_y;
    i16 vx, vy, sx, sy, row, x, y, r;
    i32 w = (i32)(map_file.w - 2) * 16, h = (i32)(map_file.h - 2) * 16;
    char name[16];
    u8 *pic;

    if (w <= 0 || h <= 0)
        return -1;
    pic = malloc((size_t)(w * h));
    if (pic == 0)
        return -1;
    map_scale = 0;
    for (vy = 1; vy < map_file.h - 1; vy += 12) {
        for (vx = 1; vx < map_file.w - 1; vx += 15) {
            map_center_x = vx + 7;
            map_center_y = vy + 6;
            compute_view_parameters();
            generate_terrain_map_region(map_corner_x, map_corner_y, map_view_size_x, map_view_size_y, -1);
            for (sy = map_corner_y; sy <= map_corner_y2; sy++)
                for (sx = map_corner_x; sx <= map_corner_x2; sx++) {
                    x = (sx - map_corner_x + map_tile_inset_x) * 16;
                    y = (sy - map_corner_y + map_tile_inset_y) * 16;
                    for (row = 0; row < 16; row++)
                        memcpy(pic + ((i32)(sy - 1) * 16 + row) * w + (i32)(sx - 1) * 16,
                               scr_orig.bits + (i32)(y + row) * scr_orig.w + x, 16);
                }
        }
    }
    map_scale = scale;
    map_center_x = cx;
    map_center_y = cy;
    compute_view_parameters();

    for (r = 0; r < 8 && map_name[r] && map_name[r] != '.'; r++)
        name[r] = map_name[r] >= 'a' && map_name[r] <= 'z' ? map_name[r] - 0x20 : map_name[r];
    name[r] = 0;
    if (r == 0)
        msc_strcpy(name, "MAP");
    msc_strcat(name, ".png");
    r = (i16)port_export_png(name, (int)w, (int)h, pic, msc_vga + 0xfc00, editor_options.export_dir,
                             editor_options.export_fallback, where, cap);
    free(pic);
    return r;
}
