/* From mapedit-decomp src/editor/map_a (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "buffer.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "screen.h"
#include "sprite.h"
#include "units.h"
/* MODULE map_a: MAPEDIT.EXE segment 0a47 (map_a.obj), file 0xba76-0xcb64, 19 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn compute_view_parameters     0x0ba76
 * fn coast_scan                  0x0bc1e
 * fn range_count                 0x0bd24
 * fn range_scan                  0x0bd40
 * fn nb_mask_p0                  0x0bd7e
 * fn nb_match_p0                 0x0bdde
 * fn is_land_seam                0x0be4e
 * fn nb_land_seam                0x0be88
 * fn nb_mask_p1                  0x0beee
 * fn nb8_mask_p1                 0x0bf4e
 * fn draw_map_icon               0x0bfc2
 * fn draw_map_tile               0x0c022
 * fn draw_map_icon2              0x0c086
 * fn draw_map_tile2              0x0c0e6
 * fn draw_seams                  0x0c14a
 * fn draw_square                 0x0c3a2
 * fn generate_terrain_map_region 0x0c7d6
 * fn generate_terrain_map        0x0ca92
 * fn generate_colony_map         0x0cb18
 * fn map_init                    0x0cb28
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl21_6787c).
 *
 * THE 1994 TERRAIN RENDERER. Windows rebuilt map drawing around sprites and a precomputed seam
 * map (SetupDitherMap, draw_tile_with_overlays ..), so only the outer functions have Windows twins:
 * compute_view_parameters (MapWin_ComputeViewParameters), generate_terrain_map_region,
 * generate_terrain_map (= GenerateTerrainMap) and generate_colony_map. Everything in between is DOS-only:
 * draw_square draws one map square into the backdrop port from the cursor globals
 * (scan_map/p1/p3 into map planes 0/1/3, scan_x/y, gen_tile_x/py) that generate_terrain_map_region
 * walks across the region; the near helpers count matching neighbours (4 orthogonal, bit
 * 8/4/2/1 = north/south/west/east) to pick the edge, river, road and coast icons.
 *
 * dos-modules.json's proposals here (draw_tile_with_overlays, terrain_tile_index, ColonistIcon,
 * pick_ship_in_chain, PieceIconDraw, FortDraw, animate_unit_move, buffer_draw_box, is_forest,
 * font_height, to_upper_char) are all wrong: none of these bodies match.
 *
 *  No volatiles: the three were length hacks
 *     standing in for five source shapes. (2) The 8-way scan body is
 *     `if ((k & 1) || sx < 0 || sy < 0) continue;` then `if (sx >= w || sy >= h) continue;`:
 *     the first `|| continue` parks the bound tests at the loop top before the decrement, the
 *     second parks the map_get body after the `jmp` (docs/msc6-codegen.md: "|| before continue"). (3) The
 *     dead `les bx,[scan_feature]` is a never-read local: `feat_b = scan_feature[off_l]` (store
 *     dropped, address kept) beside a single-use `site_f = scan_site[off_l]` folded into the
 *     vis_mask test; both keep a slot (bp-6, bp-0x1a), like the three ?: temps at bp-0x28..
 *     (4) `dx_e = ..; dy_g = ..;` in that order: only the FIRST assignment sinks, so dx sinks
 *     into nx_b and dy is loaded first into cx. (5) `cmp al,[cls_i]` is a byte compare,
 *     `(unsigned char)cc_f == cls_i`; and `tt_f = (unsigned char)(map_get(..) & 0x1f)` with
 *     `int map_get()` gives `and al,0x1f; sub ah,ah` (an unsigned char prototype folds the
 *     zero-extend into `and ax`). Then 6 pads before the function flip the two bound
 *     compares to load map_file first; the two new `continue`s count as symbols, so the
 *     compensation is 4 pads after it plus 2 before map_init (3 once generate_terrain_map_region
 *     lost its filler; a single group after draw_seams cannot serve draw_square/
 *     generate_terrain_map_region and map_init at once).
 *     The old note follows. That register-cache state the note blamed is what `volatile` buys:
 *     `volatile int dx_e`, `volatile int feat_b` and `volatile int sx_n` keep three of the
 *     stores-and-reloads MSC was folding. A greedy climb over all 19 locals picks
 *     dx_e (-> 580), then feat_b (-> 594), then cls_i (-> 601, 326 bytes) -- but sx_n as the
 *     third is the one that lands on 600 exactly, and the exact length is what matters here,
 *     because it is what let draw_square match. Pads 0..16 add nothing on top (0..3 are equal,
 *     4+ are worse). What is still left: the shipped code reloads t/tt/cc from memory after
 *     each `if (x < 0x18) x &= 7` and keeps a dead `les bx,[scan_feature]` before the scan_site[off]
 *     load, and the 8-way scan loop is laid out differently.
 * without the three volatiles (574 bytes) the residue reads as if shipped had no
 *     register cache at these sites (`and byte [t],7`, `and word [tt],7`, `push word [x]`).
 *     Byte-identical, so no lead: `t &= 7` vs `t = t & 7` (all three clamps), the clamp as
 *     `if ((t = ... & 0x1f) < 0x18) t &= 7;`, and `scan_feature[off_l];` as a statement for the
 *     dead les. Single pragma letters on/off, with and without the volatiles: nothing better
 *     ("c" off is 623-635 bytes). Both of its problems are gone: draw_seams is the right
 *     LENGTH now (see above), which fixes the 26 near-call displacements, and the swapped
 *     `mov al,[map_player_mask]; test [gen_site_tile],al` is fixed by reading the second global as an
 *     element of the first. map_player_mask (0xa89e), gen_feature_tile (0xa89f), gen_site_tile (0xa8a0),
 *     gen_base_tile (0xa8a1) and gen_map_tile (0xa8a2) are one byte array, so `(&map_player_mask)[2]` IS
 *     gen_site_tile, and an element read is not merged with a plain read of the same cell -- which
 *     is what no spelling, type or 17x17 position grid could do. The reverse element read
 *     (`(&gen_site_tile)[-2]` for map_player_mask) costs 881 bytes, and a volatile view leaves the split. (1) The clamp
 *     block in the WINDOWS order, dead `w = x1_b - x + 1;` included: `x = max; y = max; x1_b =
 *     min; y1_d = min; w = ..; h = ..; rx_p = ..; ry_b = ..`. Each clamp sinks to its next
 *     reader (x and x1_b to w, y and y1_d to h) and only then is the dead w dropped, which is
 *     how shipped lands on x1_b-min, x-max (cx), y1_d-min (ax), y-max (dx), h, rx_p, ry_b with
 *     the clamped x in cx across the y clamps. Without the w statement the clamps sink to rx_p/h
 *     instead (204). (2) The two entry tests are Windows' `if (map_corner_x2 < x) goto out;` with an
 *     `out: ;` label at the end: that loads the parameter and compares the global against it;
 *     `if (x > map_corner_x2) return;` loads the global at every pad count. (3) The label and goto
 *     shifted y1_d's position so `min(map_corner_y2, y1_d)` loaded the local first: nine block-scope
 *     `extern int pad_gty_N;` lines before `int y1_d;` (9 or 10 work) put the global first
 *     without moving a slot. The 9 block pads plus the label are 10 symbols, exactly the old
 *     pad_60..69 group after the function, which is therefore gone (generate_terrain_map,
 *     generate_colony_map and map_init unchanged; any other count breaks map_init).
 *
 * COMPILER NOTES (NEW): in this file several helpers match only with `if (c) goto out; ... out:
 * return n;` instead of `if (!c) { ... }` (nb_mask_p0 etc., is_land_seam): with the label the
 * register cache and constant knowledge die after the test, so `n += 8` is not folded to `n = 8` and
 * the fastcall parameter is re-read from its spill slot. Negation is spelled `~x + 1` (not/inc) in
 * nb8_mask_p1 and the abs() macro; `-x` gives neg, and an `x > 0 ? x : -x` macro is recognised as abs
 * (cwd/xor/sub). `t = t & 7` and `t &= 7` compile differently (memory `and byte [t],7` vs al).
 */

#include "records.h"

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))


/* compute_view_parameters -- 0a47:0006, 424 bytes */
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
/* coast_scan -- 0a47:01ae, 262 bytes */
static u8 nb_flags[4];      /* DS:0x499a: the module's BSS, first after the initialised data (where the linker's _edata label falls); no public */

/* Counts the land squares among the 8 around a water square; the bits go to scan_flags and
 * the corner table, the last orthogonal land terrain to gen_base_tile. */
i16 coast_scan(void)
{
    u8 c;
    i16 i;
    i16 j;
    u8 ter;
    i16 k;
    i16 dummy;

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
/* range_count -- 0a47:02b4, 28 bytes */

void range_count(i16 off, i16 n)
{
    u8 c;

    c = scan_map[off];
    if (c >= scan_greater && c <= scan_less)
        scan_index += n;
}
/* range_scan -- 0a47:02d0, 62 bytes */
i16 range_scan(char lo, char hi, i16 maxzoom)
{
    scan_index = 0;
    if (map_scale > maxzoom)
        goto out;
    scan_greater = lo;
    scan_less = hi;
    range_count(-map_area_size_x, 8);
    range_count(map_area_size_x, 4);
    range_count(-1, 2);
    range_count(1, 1);
out:
    return scan_index;
}
/* nb_mask_p0 -- 0a47:030e, 96 bytes */
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
/* nb_match_p0 -- 0a47:036e, 112 bytes */
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
/* is_land_seam -- 0a47:03de, 58 bytes */
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
/* nb_land_seam -- 0a47:0418, 102 bytes */
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
/* nb_mask_p1 -- 0a47:047e, 96 bytes */
i16 nb_mask_p1(i16 mask, i16 maxzoom)
{
    i16 n;

    n = 0;
    if (map_scale > maxzoom)
        goto out;
    if (scan_feature[-map_area_size_x] & mask)
        n += 8;
    if (scan_feature[map_area_size_x] & mask)
        n += 4;
    if (scan_feature[-1] & mask)
        n += 2;
    if (scan_feature[1] & mask)
        n++;
out:
    return n;
}
/* nb8_mask_p1 -- 0a47:04de, 116 bytes */
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
/* draw_map_icon -- 0a47:0552, 96 bytes */
static u8 g_icon_dx = 0;           /* DS:0x0714 (defined here: the module has no literals; module statics, no public) */
static u8 g_icon_dy = 0;           /* DS:0x0715 */

void draw_map_icon(i16 icon)
{
    void *s;

    s = tiles;
    if (map_sprite_scale >= 100)
        sprite_draw(icon, s, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        sprite_draw_scaled(icon, s, &scr_orig, gen_tile_x, gen_tile_y, map_sprite_scale);
}
/* draw_map_tile -- 0a47:05b2, 100 bytes */

void draw_map_tile(i16 tile)
{
    void *s;

    s = terrain_1;
    if (map_scale == 0)
        tile_draw(s, tile, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        tile_draw_scale(s, tile, &scr_orig, gen_tile_x, gen_tile_y, map_scale);
}
/* draw_map_icon2 -- 0a47:0616, 96 bytes */

void draw_map_icon2(i16 icon)
{
    void *s;

    s = tiles;
    if (map_sprite_scale >= 100)
        sprite_draw_black(icon, s, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        sprite_draw_scaled_black(icon, s, &scr_orig, gen_tile_x, gen_tile_y, map_sprite_scale);
}
/* draw_map_tile2 -- 0a47:0676, 100 bytes */

void draw_map_tile2(i16 tile)
{
    void *s;

    s = terrain_1;
    if (map_scale == 0)
        tile_draw_mask(s, tile, &scr_orig, g_icon_dx + gen_tile_x - 8, g_icon_dy + gen_tile_y - 15);
    else
        tile_draw_scale_mask(s, tile, &scr_orig, gen_tile_x, gen_tile_y, map_scale);
}
/* draw_seams -- 0a47:06da, 600 bytes */
#define abs(x) ((x) > 0 ? (x) : ~(x) + 1)

/* The seams between this square and its four orthogonal neighbours. */
void draw_seams(i16 hidden_only, i16 no_scan, i16 always)
{
    i16 i_h;
    i16 dx_e;
    u8 feat_b;
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
        feat_b = scan_feature[off_l];
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
/* draw_square -- 0a47:0932, 1076 bytes */

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
/* generate_terrain_map_region -- 0a47:0d66, 700 bytes */

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
    /* nine block-scope pads: they move y1_d's symbol position (mod 17) without moving its
     * slot, so `min(map_corner_y2, y1_d)` loads the global first; 10 would do as well */
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
    /* The Windows order. Each clamp sinks to its next reader: x and x1_b to the dead
     * `w = ..` (dropped after scheduling), y and y1_d to h -- which is how shipped keeps the
     * clamped x in cx across the y clamps and stores rx_p/ry_b after h. */
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
/* generate_terrain_map -- 0a47:1022, 134 bytes */

/* Windows GenerateTerrainMap. */
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
/* generate_colony_map -- 0a47:10a8, 16 bytes */
void generate_colony_map(i16 nation, i16 mode)
{
    map_colony_mode = mode;
    generate_terrain_map(nation);
    map_colony_mode = 0;
}
/* map_init -- 0a47:10b8, 60 bytes */

void map_init(void)
{
    i16 x;
    i16 y;
    i16 spare;
    i16 dummy;

    for (y = 0; y < map_file.h; y++)
        for (x = 0; map_file.w > x; x++)
            owner_set(x, y, -1);
}

/* The module's uninitialised data (communals, BSS; a one-byte communal is not word-aligned). */
u8 scan_greater;               /* DS:0x49c4 */
u8 *scan_feature;          /* DS:0x4a7e */
i16 gen_tile_x;                           /* DS:0x4b0e */
i16 gen_tile_y;                           /* DS:0x4b16 */
u8 gen_feature_tile;           /* DS:0x4c6a */
u8 gen_base_tile;              /* DS:0x4e84 */
i16 gen_variation;                        /* DS:0x4e90 */
u8 scan_less;                  /* DS:0x526e */
u8 *scan_site;             /* DS:0x52da */
u8 map_player_mask;            /* DS:0x5aaa */
u8 gen_map_tile;               /* DS:0x5acc */
u8 scan_flags;                 /* DS:0x5ad3 */
u8 *scan_map;              /* DS:0x5afa */
i16 scan_x;                               /* DS:0x5f16 */
i16 scan_y;                               /* DS:0x634c */
u8 scan_index;                 /* DS:0x6974 */
u8 gen_site_tile;              /* DS:0x6988 */

