/* mapview.c -- the map view: its geometry, the terrain renderer, refreshing it, the cursor. */
/*
 * The view shows map_view_size_x x map_view_size_y squares, from map_corner_x, map_corner_y
 * to map_corner_x2, map_corner_y2, around map_center_x, map_center_y. map_scale is the
 * zoom: at 0 the view holds 15 x 12 squares of 16 pixels, and each step doubles the squares
 * and halves their size, down to 120 x 96 squares of 2 pixels at 3, so the view is always
 * 240 x 192 pixels. A map smaller than the view is centred in it (map_tile_inset_x/y).
 *
 * Drawing goes through two buffers: the terrain is rendered into the original screen
 * (scr_orig) and copied into the work screen (scr_work) 8 rows down, below the menu bar;
 * mouse_video_update then copies the work screen to the display. The cursor square is drawn
 * in the work screen only, so restoring a square from scr_orig erases it.
 *
 * generate_terrain_map_region renders a region square by square. For each it points the scan
 * globals at the square (scan_map, scan_feature and scan_site into planes 0, 1 and 3; scan_x,
 * scan_y) and sets its pixel position (gen_tile_x at its centre column, gen_tile_y at its
 * bottom row); draw_square and its helpers read those. A square is built in layers: its
 * terrain tile, seams that dither each differing neighbour's terrain into its edges, then
 * icons from the terrain-icon sheet for forest, plowing, hills and mountains, rivers and roads.
 * Each icon is chosen by which neighbours carry the same thing, as a mask with north 8,
 * south 4, west 2 and east 1 (roads look at all eight). A water square next to land is drawn
 * as land with the coast cut out of it. The seams and the coasts are drawn only at zoom 0.
 * Squares on the map's border, and squares the viewing nation has not seen, are drawn as
 * unexplored; the editor renders for nation -1, which sees everything.
 *
 * The renderer is shared with the game. Resources and Lost City Rumours never appear in the
 * editor (both follow from a seed that is 0 here); the colony view (map_colony_mode), the
 * simplified terrain view (map_view_base_terrain) and map_high_memory_mode (a map too big
 * for the planes, which map_check refuses) are never entered.
 */
#include <stdlib.h>

#include "msc.h"
#include "assets.h"
#include "buffer.h"
#include "edit.h"
#include "editstate.h"
#include "error.h"
#include "font.h"
#include "infopanel.h"
#include "input.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mapview.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "screen.h"
#include "sprite.h"
#include "timer.h"
#include "units.h"
#include "w16types.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */

/* Derives the view from map_scale and the centre: the view's size in squares and the
 * square size in pixels; the corner squares, kept so that the view stays inside the map's
 * border; the insets that centre a map narrower or shorter than the view; the part of the
 * planes the renderer walks (map_area_*: the whole map, or a window around the view in
 * high-memory mode); the sprite scale in percent and the scroll speed. In colony mode the
 * view is 5 x 5 squares around the centre at zoom 0, and is not kept off the border. */
void compute_view_parameters(void)
{
    i16 x1;
    i16 y1;

    map_view_size_x = 15 << map_scale;
    map_view_size_y = 12 << map_scale;
    if (map_colony_mode != 0) {
        map_view_size_y = map_view_size_x = 5;
        map_scale = 0;
    }
    map_tile_size_y = map_tile_size_x = 16 >> map_scale;
    map_corner_x = map_center_x - (map_view_size_x >> 1);
    map_corner_y = map_center_y - (map_view_size_y >> 1);
    if (map_colony_mode == 0) {
        map_corner_x = min(max(map_corner_x, 1), map_file.w - map_view_size_x - 1);
        map_corner_y = min(max(map_corner_y, 1), map_file.h - map_view_size_y - 1);
    }
    map_tile_inset_y = map_tile_inset_x = 0;
    if (map_file.w - 2 < map_view_size_x) {
        map_corner_x = 1;
        map_tile_inset_x = (map_view_size_x - map_file.w + 2) >> 1;
        map_view_size_x = map_file.w - 2;
    }
    if (map_file.h - 2 < map_view_size_y) {
        map_corner_y = 1;
        map_tile_inset_y = (map_view_size_y - map_file.h + 2) >> 1;
        map_view_size_y = map_file.h - 2;
    }
    map_area_base_x = map_corner_x - 1;
    map_area_base_y = map_corner_y - 1;
    if (map_high_memory_mode != 0) {
        if (map_colony_mode == 0) {
            map_area_size_x = (15 << map_scale) + 2;
            map_area_size_y = (12 << map_scale) + 2;
        } else {
            x1 = map_area_base_x + 6;
            y1 = map_area_base_y + 6;
            map_area_size_x = min(map_file.w - 1, x1) - (map_area_base_x = max(map_area_base_x, 0)) + 1;
            map_area_size_y = min(map_file.h - 1, y1) - (map_area_base_y = max(map_area_base_y, 0)) + 1;
        }
    } else {
        map_area_size_x = map_file.w;
        map_area_size_y = map_file.h;
    }
    map_sprite_scale = 100 >> map_scale;
    map_scroll_speed = (5 << map_scale) + 5;
    map_corner_x2 = map_view_size_x + map_corner_x - 1;
    map_corner_y2 = map_view_size_y + map_corner_y - 1;
}
/* coast_scan's land code for each quarter of the square: NW, NE, SE, SW */
static u8 nb_flags[4];

/* For the water square at the scan pointers, at zoom 0: finds which of its eight neighbours
 * are land (anything but ocean and sea lane; forests count as their open terrain).
 *   - scan_flags gets bit i for direction i of move_x/move_y (N, NE, E, SE, S, SW, W, NW),
 *     scan_index the count;
 *   - nb_flags gives each quarter of the square a 3-bit code of the land at its corner: 4 for
 *     the orthogonal neighbour clockwise of the corner (N for NW, E for NE ...), 2 for the
 *     diagonal, 1 for the orthogonal neighbour anticlockwise of it;
 *   - gen_base_tile becomes the terrain of the last orthogonal land neighbour -- the land
 *     the coast is drawn on -- and gen_map_tile the terrain that shows for it. With land only
 *     on the diagonals they stay the square's own.
 * Returns the number of land neighbours; at other zooms, 0, so the water is drawn plain. */
i16 coast_scan(void)
{
    u8 c;
    i16 i;
    i16 j;
    u8 ter;
    i16 k;

    scan_flags = scan_index = 0;
    for (i = 0; i < 4; i++)
        nb_flags[i] = 0;
    if (map_scale == 0) {
        for (i = 0; i < 8; i++) {
            ter = scan_map[move_x[i] + (move_y[i] < 0 ? -map_area_size_x : 0) + (move_y[i] > 0 ? map_area_size_x : 0)] & 0x1f;
            if (ter >= 0x18)
                goto skip;
            ter &= 7;
        skip:
            c = terrain_fix_2(ter);
            if (c == 0x19 || c == 0x1a)
                goto next;
            scan_flags |= 1 << i;
            scan_index++;
            if (i & 1) {
                nb_flags[((i + 1) & 6) >> 1] |= 2;
            } else {
                gen_base_tile = ter;
                j = i >> 1;
                k = (j + 1) & 3;
                nb_flags[j] |= 4;
                nb_flags[k] |= 1;
            }
        next:
            ;
        }

    }
    gen_map_tile = terrain_fix_2(gen_base_tile);
    return scan_index;
}
/* The orthogonal neighbours whose terrain byte has a bit of mask set, as a direction mask
 * (N 8, S 4, W 2, E 1); 0 when the zoom is above maxzoom. */
i16 nb_mask_p0(i16 mask, i16 maxzoom)
{
    i16 n;

    n = 0;
    if (maxzoom < map_scale)
        goto out;
    if (scan_map[-map_area_size_x] & mask)
        n += 8;
    if (scan_map[map_area_size_x] & mask)
        n += 4;
    if (scan_map[-1] & mask)
        n += 2;
    if (scan_map[1] & mask)
        n++;
out:
    return n;
}
/* The orthogonal neighbours whose hills and mountain bits (0xa0) equal val, as a direction
 * mask; 0 when the zoom is above maxzoom. */
i16 nb_match_p0(i16 val, i16 maxzoom)
{
    i16 n;
    i16 m;

    n = 0;
    if (map_scale > maxzoom)
        goto out;
    m = 0xa0;
    if ((scan_map[-map_area_size_x] & m) == val)
        n += 8;
    if ((scan_map[map_area_size_x] & m) == val)
        n += 4;
    if ((scan_map[-1] & m) == val)
        n += 2;
    if ((scan_map[1] & m) == val)
        n++;
out:
    return n;
}
/* 1 when the square at offset off from the scan square is a forest that gets the forest
 * icon: any forest but scrub forest (the forest on desert), which has its own tile. The
 * first parameter is unused. */
i16 is_land_seam(i16 unused, i16 off)
{
    i16 res;
    i16 t;

    res = 0;
    t = scan_map[off] & 0x1f;
    if (t >= 0x18)
        goto out;
    if ((t & 7) != 1 && t > 7)
        res = 1;
out:
    return res;
}
/* The orthogonal neighbours that pass is_land_seam, as a direction mask; 0 when the zoom is
 * above maxzoom. */
i16 nb_land_seam(i16 a, i16 maxzoom)
{
    i16 n;

    n = 0;
    if (map_scale > maxzoom)
        goto out;
    if (is_land_seam(a, -map_area_size_x))
        n += 8;
    if (is_land_seam(a, map_area_size_x))
        n += 4;
    if (is_land_seam(a, -1))
        n += 2;
    if (is_land_seam(a, 1))
        n++;
out:
    return n;
}
/* All eight neighbours whose feature byte has a bit of mask set: bit i for direction i of
 * move_x/move_y (N, NE, E ... NW). 0 when the zoom is above maxzoom. */
i16 nb8_mask_p1(i16 mask, i16 maxzoom)
{
    i16 i;
    i16 res;
    i16 bit;

    res = 0;
    if (maxzoom < map_scale)
        goto out;
    bit = 1;
    for (i = 0; i < 8; i++) {
        if (scan_feature[move_x[i] + (move_y[i] != 0 ? (move_y[i] < 0 ? ~map_area_size_x + 1 : map_area_size_x) : 0)
] & mask)
            res |= bit;
        bit <<= 1;
    }
out:
    return res;
}
/* An offset for full-size icons, in pixels: draw_square sets it for the coast's quarters. */
static u8 g_icon_dx = 0;
static u8 g_icon_dy = 0;

/* Draws icon `icon` of the terrain-icon sheet on the current square in scr_orig: at full
 * size from the square's top-left corner (moved by g_icon_dx, g_icon_dy), otherwise scaled
 * to the zoom. */
void draw_map_icon(i16 icon)
{
    void *s;

    s = tiles;
    if (map_sprite_scale >= 100)
        sprite_draw(icon, s, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        sprite_draw_scaled(icon, s, &scr_orig, gen_tile_x, gen_tile_y, map_sprite_scale);
}
/* Draws terrain tile `tile` on the current square in scr_orig: at full size (moved by
 * g_icon_dx, g_icon_dy) at zoom 0, otherwise scaled. */
void draw_map_tile(i16 tile)
{
    void *s;

    s = terrain_1;
    if (map_scale == 0)
        tile_draw(s, tile, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        tile_draw_scale(s, tile, &scr_orig, gen_tile_x, gen_tile_y, map_scale);
}
/* draw_map_tile, drawing only onto pixels that are 0: the tile shows through the holes an
 * icon has left (a seam's or a coast's). */
void draw_map_tile2(i16 tile)
{
    void *s;

    s = terrain_1;
    if (map_scale == 0)
        tile_draw_mask(s, tile, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        tile_draw_scale_mask(s, tile, &scr_orig, gen_tile_x, gen_tile_y, map_scale);
}
/* The absolute value. It replaces <stdlib.h>'s abs for the rest of this file. */
#define abs(x) ((x) > 0 ? (x) : ~(x) + 1)

/* Draws the seams between the current square and its four orthogonal neighbours: for a
 * neighbour of a different terrain, edge icon 0x69 + direction (N, E, S, W) leaves a dithered
 * pattern of holes along that edge and the neighbour's tile shows through them.
 *   - A water neighbour stands for the first land square orthogonally beside it (looking W,
 *     S, E, then N), and gives no seam when there is none. With no_scan set it is taken as
 *     water, and seams only with `always` set, into an unexplored square, or when hidden.
 *     Plain water squares pass both no_scan and always.
 *   - hidden_only is for an unexplored square: only its visible neighbours seam into it,
 *     whatever their terrain.
 *   - A hidden neighbour (unseen, on the map's border or outside a colony's reach) is never
 *     skipped as matching; its real terrain is drawn into the edge.
 * gen_variation is held at 0 meanwhile and restored after. */
void draw_seams(i16 hidden_only, i16 no_scan, i16 always)
{
    i16 i_h;
    i16 dx_e;
    i16 out_k;
    i16 dy_g;
    u8 t;
    i16 k_k;
    i16 hid;
    i16 off_l;
    i16 nx_b;
    volatile i16 sx_n;
    i16 ny_c;
    u8 cls_i;
    u8 site_f;
    i16 cc_f;
    i16 sy;
    i16 tt_f;
    i16 adx_b;
    i16 save;

    save = gen_variation;
    i_h = gen_variation = 0;
    for (; i_h < 4; i_h++) {
        dx_e = card_x[i_h];
        dy_g = card_y[i_h];
        nx_b = dx_e + scan_x;
        ny_c = dy_g + scan_y;
        out_k = !on_map(nx_b, ny_c);
        if (map_colony_mode != 0) {
            adx_b = abs(nx_b - map_center_x);
            out_k |= !on_colony_map(adx_b, abs(ny_c - map_center_y), map_colony_mode);
        }
        off_l = dx_e;
        if (dy_g < 0)
            off_l = dx_e - map_area_size_x;
        if (dy_g > 0)
            off_l += map_area_size_x;
        t = scan_map[off_l] & 0x1f;
        if (t >= 0x18) ; else t &= 7;
        cls_i = terrain_fix_2(t);
        site_f = scan_site[off_l];
        hid = (map_player_mask != 0 && !(map_player_mask & site_f)) || out_k ? 1 : 0;
        if (hidden_only != 0 && hid != 0)
            continue;
        if ((cls_i == 0x19 || cls_i == 0x1a) && no_scan == 0) {
            for (k_k = 7; (cls_i == 0x19 || cls_i == 0x1a) && k_k >= 0; k_k--) {
                sx_n = move_x[k_k] + nx_b;
                sy = move_y[k_k] + ny_c;
                if ((k_k & 1) || sx_n < 0 || sy < 0)
                    continue;
                if (sx_n >= map_file.w || sy >= map_file.h)
                    continue;
                tt_f = (u8)(map_get(sx_n, sy) & 0x1f);
                if (tt_f >= 0x18) ; else tt_f = tt_f & 7;
                cls_i = terrain_fix_2(tt_f);
            }
            if (cls_i == 0x19 || cls_i == 0x1a)
                continue;
        }
        if ((cls_i == 0x19 || cls_i == 0x1a) && always == 0 && hidden_only == 0 && hid == 0)
            continue;
        cc_f = gen_map_tile & 0x1f;
        if (cc_f >= 0x18) ; else cc_f = cc_f & 7;
        cc_f = terrain_fix_2(cc_f);
        if ((u8)cc_f == cls_i && hidden_only == 0 && hid == 0)
            continue;
        draw_map_icon(i_h + 0x69);
        draw_map_tile2(cls_i);
    }
    gen_variation = save;
}
/* Renders the square at the scan pointers into scr_orig, from the bottom layer up.
 *   - Unexplored (hidden set, or unseen by map_player_mask): the unexplored icon (0x95) and,
 *     at zoom 0, the seams of the visible neighbours.
 *   - Water with no land around (or any water above zoom 0): the water tile, then at zoom 0
 *     its resource and its seams with any differing neighbour, sea lane against ocean
 *     included.
 *   - Otherwise the terrain tile -- for a coastal water square, the tile of the land
 *     neighbour coast_scan chose, forest and all -- and at zoom 0 its seams. Forests other
 *     than scrub add the forest icon (0x41 + the forest neighbours); scrub forest replaces
 *     the tile with tile 0x11 instead. Then plowing (0x96), and on land only: hills or
 *     mountains (0x31 or 0x21 + the neighbours with the same, counted up to zoom 2); a river
 *     (minor 0x11, major 1, + the river neighbours, all four when there are none). At zoom 0
 *     in the full view, the resource and a Lost City Rumour; on land in the full view, a road
 *     or settlement (0x52 + direction for each road neighbour, counted up to zoom 1; the lone
 *     road 0x51 when there are none).
 *   - A coastal water square then gets its coast: one whole-square icon (0x97 + 0-3) when the
 *     shore runs diagonally across it (land on three sides around one corner, water on the
 *     opposite three), otherwise one icon per quarter by its nb_flags code, then the water
 *     tile through the holes they leave. A river on the square adds a river
 *     mouth (0x8d major, 0x91 minor, + direction) toward each land neighbour with a river,
 *     and at zoom 0 the resource is drawn again on top.
 * `hidden` comes from generate_terrain_map_region (border and colony reach). hill_p holds
 * the square's river bits. */
void draw_square(i16 hidden)
{
    u8 water_n;
    u8 coastal_k;
    i16 m_f;
    i16 hide_j;
    i16 ua_o;
    i16 i_m;
    i16 ub_p;
    i16 k2_k;
    i16 hill_p;
    i16 n_l;
    i16 j;
    i16 k;
    i16 bit_n;
    i16 count_e;
    i16 res_e;
    i16 base_e;

    count_e = 0;
    gen_feature_tile = *scan_feature;
    gen_base_tile = *scan_map;
    gen_site_tile = *scan_site;
    gen_map_tile = terrain_fix_2(gen_base_tile);
    hide_j = (map_player_mask != 0 && !(gen_site_tile & map_player_mask)) || hidden ? 1 : 0;
    hill_p = gen_base_tile & 0xc0;
    if (hide_j != 0) {
        draw_map_icon(0x95);
        if (map_scale != 0)
            return;
        draw_seams(1, gen_map_tile == 0x19 || gen_map_tile == 0x1a ? 1 : 0, 0);
        return;
    }
    coastal_k = 0;
    water_n = 0x19;
    if (gen_map_tile == 0x19 || gen_map_tile == 0x1a) {
        water_n = gen_map_tile;
        count_e = coast_scan();
        coastal_k = 1;
    }
    if (coastal_k && count_e == 0) {
        draw_map_tile(water_n);
        if (map_scale != 0)
            return;
        res_e = resource_at(scan_x, scan_y);
        if (res_e != -1 && map_colony_mode == 0)
            draw_map_icon(res_e + 0x5a);
        draw_seams(0, 1, 1);
        return;
    }
    if (gen_map_tile < 0x18)
        base_e = gen_map_tile & 7;
    else
        base_e = gen_map_tile;
    if (base_e != 1 || !(gen_map_tile >= 8 && gen_map_tile < 0x10 || gen_map_tile >= 0x10 && gen_map_tile < 0x18))
        draw_map_tile(base_e);
    else
        draw_map_tile(0x11);
    if (map_scale == 0)
        draw_seams(0, coastal_k, 0);
    if (base_e != 1 && (gen_map_tile >= 8 && gen_map_tile < 0x10 || gen_map_tile >= 0x10 && gen_map_tile < 0x18)) {
        n_l = nb_land_seam(base_e, 3);
        draw_map_icon(n_l + 0x41);
    }
    if (gen_feature_tile & 0x40)
        draw_map_icon(0x96);
    if ((gen_base_tile & 0x20) && !coastal_k) {
        n_l = nb_match_p0(gen_base_tile & 0xa0, 2);
        draw_map_icon(gen_base_tile & 0x80 ? n_l + 0x21 : n_l + 0x31);
    }
    if ((gen_base_tile & 0x40) && !coastal_k) {
        if (gen_base_tile & 0x80)
            k = 1;
        else
            k = 0x11;
        n_l = nb_mask_p0(0x40, 3);
        if (n_l == 0)
            n_l = 0xf;
        draw_map_icon(k + n_l);
    }
    if (map_scale == 0 && map_view_base_terrain == 0) {
        res_e = resource_at(scan_x, scan_y);
        if (res_e != -1 && map_colony_mode == 0)
            draw_map_icon(res_e + 0x5a);
        if (lost_city_at(scan_x, scan_y))
            draw_map_icon(0x68);
    }
    if ((gen_feature_tile & 0xa) && !coastal_k && map_view_base_terrain == 0) {
        n_l = nb8_mask_p1(0xa, 1);
        if (n_l == 0) {
            draw_map_icon(0x51);
        } else {
            for (bit_n = 1, j = 0; j < 8; bit_n <<= 1, j++)
                if (n_l & bit_n)
                    draw_map_icon(j + 0x52);
        }
    }
    if (!coastal_k)
        return;
    m_f = -1;
    if ((scan_flags & 0xdd) == 0xc1)
        m_f = 0;
    if ((scan_flags & 0x77) == 0x07)
        m_f = 1;
    if ((scan_flags & 0x77) == 0x70)
        m_f = 2;
    if ((scan_flags & 0xdd) == 0x1c)
        m_f = 3;
    if (m_f < 0) {
        for (i_m = 0; i_m < 4; i_m++) {
            k2_k = (i_m + 1) & 3;
            g_icon_dx = (k2_k >> 1) * 8;
            g_icon_dy = (i_m >> 1) * 8;
            draw_map_icon(nb_flags[i_m] * 4 + i_m + 0x6d);
        }
        g_icon_dx = g_icon_dy = 0;
    } else {
        g_icon_dx = g_icon_dy = 0;
        draw_map_icon(m_f + 0x97);
    }
    draw_map_tile2(water_n);
    if (hill_p) {
        k2_k = hill_p & 0x80 ? 0x8d : 0x91;
        for (i_m = 0; i_m < 4; i_m++) {
            ub_p = scan_map[card_x[i_m] + (card_y[i_m] < 0 ? -map_area_size_x : 0) + (card_y[i_m] > 0 ? map_area_size_x : 0)];
            if ((ub_p & 0x40) && (ua_o = terrain_fix_2(ub_p)) != 0x19 && ua_o != 0x1a)
                draw_map_icon(k2_k + i_m);
        }
    }
    if (map_scale == 0 && map_colony_mode == 0) {
        res_e = resource_at(scan_x, scan_y);
        if (res_e != -1)
            draw_map_icon(res_e + 0x5a);
    }
}
/* Renders the squares (x, y, w, h), clipped to the view, into scr_orig as nation `nation`
 * sees them: map_player_mask becomes its bit in the site plane, or 0 (everything visible)
 * for -1. Recomputes the view first; does nothing when the region starts beyond the view.
 * Squares on the map's border, and in colony mode those outside the colony's reach, are
 * drawn hidden. The scan pointers do not advance over border squares, so a border square
 * reads the planes at the next interior square. gen_variation alternates along each row;
 * nothing reads it. */
void generate_terrain_map_region(i16 x, i16 y, i16 w, i16 h, i16 nation)
{
    u8 *p1_p;
    i16 adx_d;
    i16 ex_e;
    u8 *p0_c;
    i16 hid_o;
    i16 ey_g;
    i16 xx_f;
    i16 yy_e;
    i16 stride_m;
    i16 off_n;
    i16 rx_p;
    i16 x1_b;
    i16 dist_h;
    i16 ry_b;
    i16 y1_d;
    u8 *p3_l;

    dist_h = 0;

    if (nation >= 0)
        map_player_mask = 1 << (nation + 4);
    else
        map_player_mask = 0;
    compute_view_parameters();
    if (map_corner_x2 < x)
        goto out;
    if (map_corner_y2 < y)
        goto out;
    x1_b = w + x - 1;
    y1_d = h + y - 1;
    /* clip to the view; w is computed but not used again */
    x = max(x, map_corner_x);
    y = max(y, map_corner_y);
    x1_b = min(x1_b, map_corner_x2);
    y1_d = min(map_corner_y2, y1_d);
    w = x1_b - x + 1;
    h = y1_d - y + 1;
    rx_p = x - map_corner_x;
    ry_b = y - map_corner_y;
    if (map_high_memory_mode != 0) {
        off_n = rx_p + (ry_b + 1) * map_area_size_x + 1;
        p0_c = map + off_n;
        p1_p = feature + off_n;
        p3_l = site + off_n;
        stride_m = map_area_size_x;
    } else {
        xx_f = max(x, 1);
        yy_e = max(y, 1);
        p0_c = map_loc(xx_f, yy_e);
        p1_p = feature_loc(xx_f, yy_e);
        p3_l = site_loc(xx_f, yy_e);
        stride_m = map_file.w;
    }
    gen_tile_y = (map_tile_inset_y + ry_b + 1) * map_tile_size_y - 1;
    for (yy_e = y; yy_e <= y1_d; yy_e++) {
        scan_y = yy_e;
        scan_map = p0_c;
        scan_feature = p1_p;
        scan_site = p3_l;
        ey_g = yy_e > 0 && yy_e < map_file.h - 1 ? 1 : 0;
        if (map_colony_mode != 0)
            dist_h = abs(yy_e - map_center_y);
        gen_variation = 0;
        gen_tile_x = (map_tile_size_x >> 1) + (map_tile_inset_x + rx_p) * map_tile_size_x;
        for (xx_f = x; xx_f <= x1_b; xx_f++) {
            scan_x = xx_f;
            ex_e = xx_f > 0 && xx_f < map_file.w - 1 ? 1 : 0;
            hid_o = !ex_e || !ey_g ? 1 : 0;
            if (map_colony_mode != 0) {
                adx_d = abs(xx_f - map_center_x);
                hid_o |= !on_colony_map(adx_d, dist_h, map_colony_mode);
            }
            draw_square(hid_o);
            gen_tile_x += map_tile_size_x;
            if (ex_e && ey_g) {
                scan_map++;
                scan_feature++;
                scan_site++;
            }
            gen_variation ^= 1;
        }
        gen_tile_y += map_tile_size_y;
        if (ey_g) {
            p0_c += stride_m;
            p1_p += stride_m;
            p3_l += stride_m;
        }
    }
out:
    ;
}
/* Renders the whole view into scr_orig for nation `nation` (-1: everything). When the map
 * does not fill the view, the original screen is first covered with the backdrop tile
 * (aligned with the screen below the menu bar), or black without one. */
void generate_terrain_map(i16 nation)
{
    compute_view_parameters();
    if (map_tile_inset_x != 0 || map_tile_inset_y != 0) {
        if (main_tile != 0)
            buffer_tile(scr_orig, *main_tile, 0, 0, scr_orig.w, scr_orig.h, 0, -8);
        else
            buffer_fill(0, scr_orig);
    }
    generate_terrain_map_region(map_corner_x, map_corner_y, map_view_size_x, map_view_size_y, nation);
}

/* The renderer's state: where it is and what the current square holds. */
u8 *scan_feature;          /* the current square in the feature plane (1) */
i16 gen_tile_x;                           /* the current square's centre column in scr_orig */
i16 gen_tile_y;                           /* the current square's bottom row in scr_orig */
u8 gen_feature_tile;           /* the current square's feature byte */
u8 gen_base_tile;              /* its terrain byte (a coast's: the land it is drawn on) */
i16 gen_variation;                        /* alternates along each row; not read */
u8 *scan_site;             /* the current square in the site plane (3) */
u8 map_player_mask;            /* the viewing nation's site-plane bit; 0 sees everything */
u8 gen_map_tile;               /* the terrain that shows: terrain_fix_2(gen_base_tile) */
u8 scan_flags;                 /* coast_scan's land neighbours, a bit per direction */
u8 *scan_map;              /* the current square in the terrain plane (0) */
i16 scan_x;                               /* the current square */
i16 scan_y;
u8 scan_index;                 /* coast_scan's count of land neighbours */
u8 gen_site_tile;              /* the current square's site byte */

/* Redraws the squares (x, y, w, h) of the view: clips them, renders them for nation -1,
 * copies them to the work screen and the display, and redraws the minimap. */
void draw_map_region_2(i16 x, i16 y, i16 w, i16 h)
{
    conform_to_view_size(&x, &y, &w, &h);
    generate_terrain_map_region(x, y, w, h, -1);
    update_terrain_region(x, y, w, h);
    show_map_region(x, y, w, h);
    show_mini(1, -1);
}
/* Redraws the whole view: a black line just outside it (along its top and right edges; the
 * rest is off the screen), the terrain for nation -1, the copy to the work screen and the
 * display, and the minimap. */
void draw_map_2(void)
{
    buffer_draw_box(-1, 7, map_pixel_size_x, scr_work, map_pixel_size_y + 8, 0);
    generate_terrain_map(-1);
    update_terrain();
    show_map();
    show_mini(1, -1);
}

/* Clips the region (*x, *y, *w, *h) to the view's squares; an empty result has w or h 0. */
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
/* Copies the squares (x, y, w, h) of the view from scr_orig into the work screen, 8 rows
 * down. The region must already lie inside the view. */
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
/* Copies the whole view from scr_orig into the work screen. */
void update_terrain(void)
{
    buffer_rect_copy_2(0, 0, 0, scr_orig, scr_work, 8, map_pixel_size_x, map_pixel_size_y);
}
/* Copies the view from the work screen to the display. */
void show_map(void)
{
    mouse_video_update(0, 8, 0, 8, map_pixel_size_x, map_pixel_size_y);
}
/* Copies the squares (x, y, w, h) of the view from the work screen to the display. */
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

/* Redraws the whole screen: clears it, then draws the menu bar, the map with the minimap,
 * and the info window. */
void main_screen_refresh(void)
{
    buffer_fill(0, scr_work);
    mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
    menu_draw_bar(menu_bar, 0, 1);
    draw_map_2();
    info_window_draw();
}

/* Toggles the map cursor (cursor_on) and redraws the info window. When the cursor's square
 * is in the view, restores the square from scr_orig, draws the cursor icon for the zoom
 * (icon 0x13 + map_scale) over it if the cursor is now on, and shows the square. */
void cursor_update(void)
{
    i16 sx;
    i16 sy;

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

Menu *menu_bar = 0;                         /* the editor's menu bar (construct_mapedit_menu) */
