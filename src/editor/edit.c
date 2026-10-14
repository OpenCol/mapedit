/* edit.c -- the editing commands: painting, picking up, the brush, the cursor and the view. */
/*
 * A square's terrain byte (plane 0) holds a terrain id in its low five bits (TERRAIN_*) and
 * the hills, mountain and river bits above them (MAP0_*). The selection lives in editstate.c
 * and is set by Map Tile Select (tilepicker.c). It is one of two kinds:
 *
 *   - a terrain: selected_terrain is its id; selected_physical_and_mask says which of the
 *     square's upper bits survive the paint (0xff, or 0x40 for ocean and sea lane, which keep
 *     only the river bit); the or mask is 0.
 *   - a feature (selected_right_mouse_change set): the square keeps its terrain; the and mask
 *     clears the feature's group of bits and the or mask sets the feature. "Picking up" with
 *     a feature selected erases it instead (the and mask alone).
 *
 * change_map edits one square, fill_map a brush of (2 * fill_radius + 1) squares a side. The
 * other commands move the cursor and the view, change the zoom, create a blank map, and show
 * the continents Find Continents numbers. Each one redraws what it changed.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "editstate.h"
#include "font.h"
#include "menu.h"
#include "buffer.h"
#include "edit.h"
#include "error.h"
#include "fileio.h"
#include "game.h"
#include "infopanel.h"
#include "input.h"
#include "loader.h"
#include "map.h"
#include "mapview.h"
#include "mem.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "screen.h"
#include "sprite.h"

/* Applies the selection to square (x, y). Squares on the map's border or outside the visible
 * view are ignored.
 *
 * pickup 0 paints: a terrain replaces the square's terrain id (keeping the upper bits its and
 * mask allows), a feature is added. pickup nonzero with a feature selected erases that
 * feature; with a terrain selected it makes the square's terrain the selection instead (ocean
 * and sea lane with and mask 0x40, anything else 0xff) and redraws the info window. A hilly
 * square's low bits are picked up as if they were a terrain id.
 *
 * Ocean and sea-lane squares are never given a hills or mountain feature. With coastline
 * protection on they are left alone altogether: the test also asks for selected_terrain >= 0,
 * which every selection satisfies. Sets map_changes when the byte changes; does not redraw. */
void change_map(register i16 x, register i16 y, i16 pickup)
{
    u8 *p;
    u8 b;
    u8 t;
    u8 t2;

    if (x < 1)
        return;
    if (y < 1)
        return;
    if (map_file.w - 1 <= x)
        return;
    if (map_file.h - 1 <= y)
        return;
    if (map_corner_x > x)
        return;
    if (map_corner_x2 < x)
        return;
    if (map_corner_y > y)
        return;
    if (map_corner_y2 < y)
        return;
    p = map_loc(x, y);
    b = *p;
    t2 = t = b & 0x1f;
    if (pickup == 0 || selected_right_mouse_change) {
        if (!((t == 0x19 || t == 0x1a) && coastline_protect && selected_terrain >= 0)) {
            if (!((t == 0x19 || t == 0x1a) && (selected_physical_or_mask & 0x20))) {
                b &= selected_physical_and_mask;
                if (pickup == 0)
                    b |= selected_physical_or_mask;
            }
            if (selected_terrain >= 0 && pickup == 0)
                t = selected_terrain;
        }
    }
    if (pickup && selected_right_mouse_change == 0) {
        if (t == 0x19 || t == 0x1a) {
            selected_terrain = t;
            selected_physical_and_mask = 0x40;
        } else {
            selected_terrain = t;
            selected_physical_and_mask = 0xff;
        }
        selected_physical_or_mask = 0;
        info_window_draw();
        return;
    }
    b &= 0xe0;
    b |= selected_right_mouse_change ? t2 : t;
    if (*p != b) {
        *p = b;
        map_changes = 1;
    }
}

/* Applies change_map to every square of the brush centred on (x, y) -- 1, 3 or 5 squares a
 * side for fill_radius 0, 1 or 2 -- then redraws the brush with a one-square margin, since
 * the neighbours' coasts and seams may change too. */
void fill_map(i16 x, i16 y, i16 pickup)
{
    i16 x0;
    i16 y0;
    i16 size;
    i16 xx;
    i16 yy;

    x0 = x - fill_radius;
    y0 = y - fill_radius;
    size = fill_radius * 2 + 1;
    for (yy = y0; yy < y0 + size; yy++)
        for (xx = x0; xx < x0 + size; xx++)
            if (on_map(xx, yy))
                change_map(xx, yy, pickup);
    draw_map_region_2(x0 - 1, y0 - 1, size + 2, size + 2);
}

/* Centres the view on (x, y), if it is on the map, and redraws the map. In the editor
 * (game_mode is always 1) the cursor moves there too; with flag set it is erased from its
 * old square first and drawn on the new one (the redraw that follows paints over it, so it
 * reappears at the next blink). */
void set_center(register i16 x, register i16 y, i16 flag)
{
    if (on_map(x, y)) {
        map_center_x = x;
        map_center_y = y;
        if (game_mode == 1) {
            if (flag && cursor_on)
                cursor_update();
            cursor_x = x;
            cursor_y = y;
            if (flag) {
                cursor_on = 0;
                cursor_update();
            }
        }
        draw_map_2();
    }
}

/* Re-centres the view on (x, y) when the rectangle (x, y)-(x2, y2) comes within 2 squares of
 * an edge of the view and the view can still scroll that way. flag goes to set_center.
 * Returns 1 when it re-centred, else 0. */
i16 possibly_center(i16 x, i16 y, i16 x2, i16 y2, i16 flag)
{
    i16 need;
    i16 maxx;
    i16 maxy;
    i16 minx;
    i16 miny;
    register i16 result;
    register i16 xr;

    xr = x;
    need = 0;
    maxx = ((x2) >= (xr) ? (x2) : (xr));
    miny = ((y2) <= (y) ? (y2) : (y));
    maxy = ((y2) >= (y) ? (y2) : (y));
    minx = ((x2) <= (xr) ? (x2) : (xr));
    if (minx < map_corner_x + 2 && map_corner_x > 1)
        need = 1;
    result = need;
    if (miny < map_corner_y + 2 && map_corner_y > 1)
        result = 1;
    if (map_corner_x2 - 2 < maxx && map_file.w - 2 > map_corner_x2)
        result = 1;
    if (map_corner_y2 - 2 < maxy && map_file.h - 2 > map_corner_y2)
        result = 1;
    if (result)
        set_center(xr, y, flag);
    return result;
}

/* Moves the cursor by (dx, dy) unless that would put it on or past the map's border, and
 * draws it. Before the move the view may re-centre -- on the cursor's OLD square, as the
 * original does, so the view trails the cursor by one square. */
void move_cursor(i16 dx, i16 dy)
{
    i16 ox;
    i16 oy;
    register i16 nx;
    register i16 ny;
    i16 ok;

    ok = 1;
    ox = cursor_x;
    oy = cursor_y;
    nx = ox + dx;
    ny = oy + dy;
    if (nx < 1 || ny < 1)
        ok = 0;
    if (map_file.w - 1 <= nx || map_file.h - 1 <= ny)
        ok = 0;
    if (ok) {
        possibly_center(ox, oy, ox, oy, 1);
        cursor_x = nx;
        cursor_y = ny;
        cursor_update();
    }
}

/* The View menu's "view mode" command: game_mode back to 1, which it always is. */
void set_view_mode(void)
{
    game_mode = 1;
}

/* Sets the zoom: level 0 (16-pixel squares) to 3 (2-pixel squares), clamped. Recomputes the
 * view and re-centres it on the cursor if that is now near an edge, else redraws it. */
void set_zoom_level(register i16 level)
{
    level = max(level, 0);
    level = min(level, 3);
    map_scale = level;
    compute_view_parameters();
    if (possibly_center(cursor_x, cursor_y, cursor_x, cursor_y, 0) == 0)
        draw_map_2();
}

/* Creates a blank map: asks for its name (offering UNTITLED.MP; the extension becomes .MP,
 * the name upper case), makes it map_name and creates a 58 x 72 map of ocean. Sets map_changes, so
 * leaving asks to save it. Returns 0 when the map was created, 1 when the name was cancelled
 * or the map could not be made. wide and high are written and never read. */
i16 create_me(void)
{
    i16 high;
    i16 wide;
    char buf[0x50];
    register i16 result;

    result = 1;
    if (popup_ask("MAPEDIT", "NEWNAME", "UNTITLED.MP", 0x14) == 0) {
        msc_strcpy(buf, popup_text_buffer);
        fileio_new_ext(buf, buf, "MP");
        *(volatile i16 *)&wide = 0x3a;
        *(volatile i16 *)&high = 0x48;
        msc_strcpy(map_name, buf);
        if (create_blank_map(0x3a, 0x48) == 0) {
            map_changes = 1;
            result = 0;
        }
    }
    return result;
}

/* Find Continents: numbers the land masses and oceans into the continent plane
 * (map_find_continents, with a 65,000-byte scratch block lent through sprite_base) and warns
 * when there are more than 15 of either. Then it shows the result straight on the screen,
 * one pixel per square from the top-left corner, each continent in the palette colour of its
 * number (the plane's low nibble) and water black, waits for a key or a click and restores the
 * screen from the work buffer. port[1] wraps the continent plane; port[0] is a copy that is
 * masked for display. Sets map_changes, since the continent numbers are saved with the map.
 * Does nothing when the scratch block cannot be had. */
void continent_check(void)
{
    i16 flags;
    i16 y;
    i16 x;
    u8 *p;
    DosPort port[2];

    sprite_base = mem_get(0xfde8L);
    if (sprite_base != 0) {
        flags = map_find_continents();
        mem_free(sprite_base);
        sprite_base = 0;
        if (flags & 1)
            popup_box("MAPEDIT", "CONTINENTS1", 0);
        if (flags & 2)
            popup_box("MAPEDIT", "CONTINENTS2", 0);
        port[1].w = map_file.w;
        port[1].h = map_file.h;
        port[1].bits = continent;
        buffer_init(&port[0], map_file.w, map_file.h);
        buffer_rect_copy(0, 0, map_file.w, port[1], port[0], map_file.h);
        for (y = 0; y < map_file.h; y++) {
            for (x = 0; x < map_file.w; x++) {
                p = (u8 *)port[0].bits + y * map_file.w + x;
                *p &= 0xf;
                if ((map_get(x, y) & 0x1f) == 0x19 || (map_get(x, y) & 0x1f) == 0x1a)
                    *p = 0;
            }
        }
        buffer_rect_copy(0, 0, map_file.w, port[0], scr_video, map_file.h);
        event_wait();
        mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
        buffer_free(&port[0]);
        map_changes = 1;
    }
}
