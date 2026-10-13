/* mapview.c -- the map view on screen: refreshing it, and the blinking cursor square. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "assets.h"
#include "buffer.h"
#include "edit.h"
#include "error.h"
#include "infopanel.h"
#include "input.h"
#include "map.h"
#include "mapedit.h"
#include "mapview.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "screen.h"
#include "sprite.h"
#include "w16types.h"

void main_screen_refresh(void)
{
    buffer_fill(0, scr_work);
    mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
    menu_draw_bar(menu_bar, 0, 1);
    draw_map_2();
    info_window_draw();
}

/* Toggles the cursor: redraws the info window, restores the square from the backdrop and
 * draws the cursor icon (0x13 + zoom) over it when it is on. */
void cursor_update(void)
{
    i16 cx;
    i16 sx;
    i16 cy;
    i16 sy;
    i16 miny;
    i16 maxx;

    info_window_draw();
    cursor_on = !cursor_on;
    if (map_corner_x <= cursor_x && map_corner_x + map_view_size_x > cursor_x
        && map_corner_y <= cursor_y && map_corner_y + map_view_size_y > cursor_y) {
        sy = (cursor_y - map_corner_y + map_tile_inset_y) * map_tile_size_y;
        sx = (cursor_x - map_corner_x + map_tile_inset_x) * map_tile_size_x;
        buffer_rect_copy_2(sx, sy, sx, scr_orig, scr_work, sy + 8, map_tile_size_x, map_tile_size_y);
        if (cursor_on)
            sprite_draw(map_scale + 0x13, icons, &scr_work, sx, sy + 8);
        mouse_video_update(sx, sy + 8, sx, sy + 8, map_tile_size_x, map_tile_size_y);
    }
}
