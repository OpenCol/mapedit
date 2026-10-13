/* edit.c -- editing the map: the selection, painting and picking up, the brush, undo, the cursor and the view. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

i16 map_changes = 0;                            /* DS:0x0046 */

i16 small_map_needs_update = 0;                 /* DS:0x0048 */

i16 fill_radius = 0;                            /* DS:0x004a */

i16 map_selected = 0;                           /* DS:0x004c */

i16 coastline_protect = 0;                      /* DS:0x004e */

i16 undo_available = 0;                         /* DS:0x0050 */

i16 undo_active = 0;                            /* DS:0x0052 */

i16 create_me_now = 0;                          /* DS:0x0054 */

/* 11 unused declarations before selected_terrain and 6 after it set its own position (mod 17)
 * for parse_spot's `cmp [bp-0xa],al` without moving any other symbol. */
char selected_terrain = 0x19;                   /* DS:0x0056 */

u8 selected_physical_and_mask = 0x40;/* DS:0x0057 */

u8 selected_physical_or_mask = 0;    /* DS:0x0058 */

char selected_right_mouse_change = 0;           /* DS:0x0059 */

void *map_undo_memory = 0;                  /* DS:0x005a */

i16 map_undo_ems_handle = -1;                   /* DS:0x005e */

i16 cursor_x;                             /* DS:0x4b52 */

i16 cursor_y;                             /* DS:0x4b54 */

/* Copies the terrain plane (0x2ee0 bytes) into the undo buffer. Both have a dead
 * `result = -1` (the `mov ax,-1`) and 6 more bytes of unused frame. */
void make_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_available) {
        msc__fmemcpy(map_undo_memory, map, 0x2ee0);
        undo_active = 1;
    }
}

/* perform_undo -- 0000:077e, 40 bytes */
void perform_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_active)
        msc__fmemcpy(map, map_undo_memory, 0x2ee0);
}

/* Paints the selected terrain (and/or masks) onto square (x, y) inside the view, or with
 * pickup picks the square's terrain up as the selection. Ocean squares are left alone when
 * the coastline protect is on and a terrain is selected. */
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

/* Paints (or picks up) a square of fill_radius around (x, y) and redraws it. */
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

/* Centres the view on (x, y); in the editor (game_mode 1) the cursor goes there too, with
 * the cursor toggled off and on around the move when flag is set. */
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

/* Re-centres on (x, y) when the rectangle (x, y)-(x2, y2) comes within 2 squares of the
 * view's edge and the view can still move that way. Returns 1 when it did. */
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

/* move_cursor -- 0000:1514, 102 bytes */
/* Moves the cursor by (dx, dy) when that stays inside the map's border. */
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

/* set_view_mode -- 0000:157a, 8 bytes */
void set_view_mode(void)
{
    game_mode = 1;
}

void set_zoom_level(register i16 level)
{
    level = max(level, 0);
    level = min(level, 3);
    map_scale = level;
    compute_view_parameters();
    if (possibly_center(cursor_x, cursor_y, cursor_x, cursor_y, 0) == 0)
        draw_map_2();
}

/* A new blank map: asks for its name (UNTITLED.MP), 0x3a x 0x48. 0 when created. Plain flags
 * (the dead wide/high stores stay, as volatile stores; the call takes the literals). */
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

/* Find Continents: numbers the land masses and oceans (map_find_continents, with a 0xfde8
 * byte scratch block in sprite_base), warns when there are more than 15 of either, and shows
 * the continent plane (low nibble, oceans black) on the screen until a key or click.
 * The two ports are one array (port[0], the aliased work buffer, sorts deepest); the plane
 * port's stores are written w, h, bits to come out h, bits, w ("elg" reverses them). */
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
