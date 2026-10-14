/* infopanel.c -- the information window at the right of the map.
 *
 * The window is 79 x 150 pixels at (241, 50) on the work screen, below the minimap. It shows
 * the map's size, the cursor's position, the terrain under the cursor, the selected terrain
 * or feature with the shift-click legend, the fill radius and whether coast protection is on.
 * info_window_draw redraws all of it, in the small menu font, and copies the window to the
 * screen.
 *
 * A map square's byte keeps the terrain in its low five bits, a river in bit 0x40 and hills or
 * mountains in bit 0x20; bit 0x80 picks the major river and the mountains. The feature words
 * ("Forest", "River", "Major River", "Minor River", "Unexplored") are terrain_names[0..4],
 * from the OTHER_NAMES section of NAMES.TXT.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
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

/* Clears the info window to the backdrop tile (colour 0x22 when there is none) and draws its
 * frame in colour 0. */
void info_window_clear(void)
{
    if (main_tile == 0)
        buffer_rect_fill(0xf1, 0x32, 0x4f, scr_work, 0x96, 0x22);
    else
        buffer_tile(scr_work, *main_tile, 0xf1, 0x32, 0x4f, 0x96, 0, 0);
    buffer_draw_box(0xf0, 0x31, 0x140, scr_work, 0xc8, 0);
}

/* Copies the info window from the work screen to the video screen. */
void info_window_update(void)
{
    mouse_video_update(0xf1, 0x32, 0xf1, 0x32, 0x4f, 0x96);
}

/* Writes "(name)" for terrain t at (*x, *y) and moves *y down a line. The forested terrains
 * (8 to 0x17) get the word "Forest" (terrain_names[0]) after the name. */
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

/* The same for the feature word terrain_names[n]. */
void info_terrain_2(i16 n, i16 *x, i16 *y)
{
    char buf[0x50];

    msc_strcpy(buf, "(");
    msc__fstrcat(buf, strings(terrain_names[n]));
    msc_strcat(buf, ")");
    write_string(buf, *x, *y);
    *y += menu_font->height + 1;
}

/* Redraws the whole info window and puts it on the screen. The terrain under the cursor is
 * followed by its river, if any. The selection is shown as a picture and a name: a feature
 * (selected_right_mouse_change set) as its sprite from the PHYS0 series, a terrain as its
 * tile, a forested one as the unforested tile with the forest sprite over it. A feature's
 * legend says the shift-clicks add and remove it, a terrain's that they paint and pick up.
 *
 * `flag` is always 1, so the branch that would name an unexplored selection never runs, and
 * `type` is computed but not used; both are so in the original. */
void info_window_draw(void)
{
    char tmp[0x50];
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
