/* infopanel.c -- the information window at the right of the map: size, cursor, the terrain under it, the selection, the brush. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "editstate.h"
#include "assets.h"
#include "buffer.h"
#include "edit.h"
#include "error.h"
#include "font.h"
#include "infopanel.h"
#include "loader.h"
#include "map.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "screen.h"
#include "sprite.h"
#include "text.h"
#include "theme.h"

/* The info window at (0xf1, 0x32), 0x4f x 0x96: the backdrop tile or colour 0x22, framed. */
void info_window_clear(void)
{
    if (main_tile == 0)
        buffer_rect_fill(0xf1, 0x32, 0x4f, scr_work, 0x96, 0x22);
    else
        buffer_tile(scr_work, *main_tile, 0xf1, 0x32, 0x4f, 0x96, 0, 0);
    buffer_draw_box(0xf0, 0x31, 0x140, scr_work, 0xc8, 0);
}

/* info_window_update -- 0000:082c, 22 bytes */
void info_window_update(void)
{
    mouse_video_update(0xf1, 0x32, 0xf1, 0x32, 0x4f, 0x96);
}

/* Writes "(terrain name)" at (*x, *y) and moves *y down a line. Forests (8-0x17) get the
 * word terrain_names[0] after the name. */
void info_terrain(i16 t, i16 *x, i16 *y)
{
    char buf[0x50];

    msc_strcpy(buf, "(");
    msc__fstrcat(buf, strings(terrain[t].name));
    if (t >= 8 && t < 0x18) {
        space_string(buf);
        msc__fstrcat(buf, strings(terrain_names[0]));
    }
    msc_strcat(buf, ")");
    write_string(buf, *x, *y);
    *y += menu_font->height + 1;
}

/* info_terrain_2 -- 0000:08e2, 108 bytes */
/* The same for a feature word (terrain_names[n]). */
void info_terrain_2(i16 n, i16 *x, i16 *y)
{
    char buf[0x50];

    msc_strcpy(buf, "(");
    msc__fstrcat(buf, strings(terrain_names[n]));
    msc_strcat(buf, ")");
    write_string(buf, *x, *y);
    *y += menu_font->height + 1;
}

/* 2 pads: the operand order of the `selected_terrain != terr` byte compare (mod 17) */
/* The info window: map size, cursor, the terrain under the cursor (with its hills/mountains),
 * the selected terrain or feature with its tile and the shift-click legend, the fill radius
 * and the coastline protect state. */
void info_window_draw(void)
{
    char tmp[0x50];
    i16 type;
    i16 x;
    i16 y;
    i16 flag;
    i16 terr;
    i16 under;

    info_window_clear();
    y = 0x33;
    x = 0xf2;
    flag = 1;
    msc_sprintf(tmp, "Size: (%d, %d)", map_file.w, map_file.h);
    write_string(tmp, x, y);
    y += menu_font->height + 1;
    msc_sprintf(tmp, "Curs: (%d, %d)", cursor_x, cursor_y);
    write_string(tmp, x, y);
    y += menu_font->height + 6;
    write_string("Terrain at cursor:", x, y);
    y += menu_font->height + 1;
    under = map_get(cursor_x, cursor_y);
    type = (u8)(under & 0x1f);
    info_terrain(terrain_type(under), &x, &y);
    if (under & 0x40) {
        if (under & 0x80)
            info_terrain_2(2, &x, &y);
        else
            info_terrain_2(3, &x, &y);
    }
    y += 0xa;
    write_string("Selected:", x, y);
    y += menu_font->height + 1;
    if (selected_right_mouse_change) {
        if (selected_physical_or_mask & 0x40) {
            if (selected_physical_or_mask & 0x80)
                sprite_draw(4, tiles, &scr_work, x, y);
            else
                sprite_draw(0x14, tiles, &scr_work, x, y);
        } else if (selected_physical_or_mask & 0x20) {
            if (selected_physical_or_mask & 0x80)
                sprite_draw(0x24, tiles, &scr_work, x, y);
            else
                sprite_draw(0x34, tiles, &scr_work, x, y);
        }
    } else {
        terr = (selected_terrain <= 7 || selected_terrain == 0x11 || selected_terrain >= 0x18)
               ? selected_terrain : selected_terrain & 7;
        tile_draw(terrain_1, terr, &scr_work, x, y);
        if (selected_terrain != (char)terr)
            sprite_draw(0x41, tiles, &scr_work, x, y);
    }
    y += 0x14;
    if (flag == 0)
        info_terrain_2(4, &x, &y);
    else if (selected_physical_or_mask & 0x20) {
        if (selected_physical_or_mask & 0x80)
            info_terrain(0x1b, &x, &y);
        else
            info_terrain(0x1c, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Add", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Remove", x, y);
    } else if (selected_physical_or_mask & 0x40) {
        if (selected_physical_or_mask & 0x80)
            info_terrain_2(2, &x, &y);
        else
            info_terrain_2(3, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Add", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Remove", x, y);
    } else {
        info_terrain(selected_terrain, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Paint", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Pick up", x, y);
    }
    y += 0x14;
    msc_sprintf(tmp, "Fill radius: %d", fill_radius);
    write_string(tmp, x, y);
    y += menu_font->height + 1;
    if (coastline_protect)
        write_string("Coast Protect: ON", x, y);
    else
        write_string("Coast Protect: OFF", x, y);
    info_window_update();
}
