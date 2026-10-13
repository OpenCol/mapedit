/* Imported from mapedit-decomp src/editor/map by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE map: MAPEDIT.EXE segment 02ab (map.obj), file 0x40be-0x46b6, 26 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn on_map         0x040be
 * fn on_colony_map  0x040f0
 * fn on_screen      0x04174
 * fn map_loc        0x041aa
 * fn map_get        0x041c2
 * fn feature_loc    0x041de
 * fn feature_get    0x041f6
 * fn feature_set    0x04212
 * fn continent_loc  0x04248
 * fn continent_get  0x04260
 * fn continent_at   0x0427e
 * fn continent_set  0x04290
 * fn owner_of       0x042b4
 * fn owner_set      0x042dc
 * fn land_region_at 0x04354
 * fn site_loc       0x04394
 * fn site_get       0x043ac
 * fn is_unit        0x043c8
 * fn is_colony      0x043f6
 * fn is_village     0x04430
 * fn is_city        0x0446a
 * fn is_anything    0x04498
 * fn is_hostile     0x044ba
 * fn resource_at    0x04508
 * fn lost_city_at   0x045f0
 * fn terrain_fix_2  0x0466c
 *
 * THE MAP PLANES (root code, frame 037f, from offset 0xa). Windows: the same functions in. The four planes are far byte arrays, CodeView names map, feature,
 * continent, site (DS:0x4a8..0x4b4; VICEROY's g_plane0..3), indexed y * map_file.w + x:
 * plane 0 terrain, plane 1 features (MAP1_*), plane 2 continent (low nibble) and owner (high
 * nibble), plane 3 the sight/visit bits. map_file (DS:0x4b12) is the .MP header {w, h, ..}.
 * DOS never checks map_index_in_range and never locks a handle (Windows: MSHLock).
 * The editor-only functions: on_screen tests the visible window (map_corner_x..x2, y..y2);
 * is_unit / is_village / is_city read the feature bits then the owner nibble (MAP1_UNIT; a
 * settlement owned by a tribe, 4 and up; any settlement); is_hostile is Windows' without the
 * map_index_in_range test, over europe[me].rel[n] (europe is DS:0x4e80, the Nation array).
 */

#include "terrain.h"
#include "records.h"

#define ABS(x) ((x) > 0 ? (x) : ~(x) + 1)

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern MapSize map_file;                  /* DS:0x4b12: the .MP header, w then h */
/* on_map -- 02ab:000e, 50 bytes */
i16 on_map(i16 x, i16 y)
{
    i16 r;

    r = 1;
    if (x < 1 || y < 1 || map_file.w - 1 <= x || map_file.h - 1 <= y)
        r = 0;
    return r;
}
/* on_colony_map -- 02ab:0040, 132 bytes */
/* Is a step of (dx, dy) short enough for the given reach: mode 1 only orthogonal neighbours
 * (|dx| + |dy| <= 1), mode 2 adds diagonals, mode 3 a distance of 2 by |dx| + |dy|, any other
 * mode also the squares two away in one direction. DOS-only (no Windows twin found). */
i16 on_colony_map(i16 dx, i16 dy, i16 mode)
{
    i16 r;

    dx = ABS(dx);
    dy = ABS(dy);
    r = dx + dy <= 1;
    if (mode == 1)
        goto out;
    if (dx < 2 && dy < 2)
        r |= 1;
    if (mode == 2)
        goto out;
    r |= dx + dy <= 2;
    if (mode == 3)
        goto out;
    if (dx < 2 || dy < 2)
        r |= 1;
out:
    return r;
}
/* on_screen -- 02ab:00c4, 54 bytes */
/* Fifteen unused declarations set on_screen's operand order (mod 17): the last compare is
 * `cmp ax,[map_corner_y2]`, the other three `cmp [g],ax`. Sixteen more after it keep the rest
 * of the module where fourteen had it. */
extern i16 pad_os_1, pad_os_2, pad_os_3, pad_os_4, pad_os_5, pad_os_6, pad_os_7, pad_os_8;
extern i16 pad_os_9, pad_os_10, pad_os_11, pad_os_12, pad_os_13, pad_os_14, pad_os_15;
extern i16 map_corner_x;                  /* DS:0x49f2 */
extern i16 map_corner_x2;                 /* DS:0x6026 */
extern i16 map_corner_y;                  /* DS:0x49f4 */
extern i16 map_corner_y2;                 /* DS:0x603e */

/* Is (x, y) inside the visible map window (map_corner_x..x2, y..y2 inclusive)? */
i16 on_screen(i16 x, i16 y)
{
    i16 r;

    r = 1;
    if (x < map_corner_x || x > map_corner_x2)
        r = 0;
    if (y < map_corner_y || y > map_corner_y2)
        r = 0;
    return r;
}
/* map_loc -- 02ab:00fa, 24 bytes */
extern i16 pad_os_16, pad_os_17, pad_os_18, pad_os_19, pad_os_20, pad_os_21, pad_os_22, pad_os_23;
extern i16 pad_os_24, pad_os_25, pad_os_26, pad_os_27, pad_os_28, pad_os_29, pad_os_30, pad_os_31;

/* The map module's data, defined here (the 23 land at DS:0x049e-0x0518 in this order, ahead of
 * owner_set's "Illegal entry into village": MSC emits a global after the literals of the
 * function that follows its definition). */
i16 map_file_version = 0;                 /* DS:0x049e */
char map_ext[4] = "mp";                   /* DS:0x04a0 */
i16 map_error = 0;                        /* DS:0x04a4 */
i16 map_high_memory_mode = 0;             /* DS:0x04a6 */
/* 13 unused declarations restore the symbol count (mod 17) the four definitions above moved. */
extern i16 pad_d1_0;
extern i16 pad_d1_1;
extern i16 pad_d1_2;
extern i16 pad_d1_3;
extern i16 pad_d1_4;
extern i16 pad_d1_5;
extern i16 pad_d1_6;
extern i16 pad_d1_7;
extern i16 pad_d1_8;
extern i16 pad_d1_9;
extern i16 pad_d1_10;
extern i16 pad_d1_11;
extern i16 pad_d1_12;
u8 *map = 0;               /* DS:0x04a8, plane 0: terrain */

/* 2 unused declarations set map_loc's operand order (mod 17). */
extern i16 pad_map_loc_0, pad_map_loc_1;
u8 *map_loc(i16 x, i16 y)
{
    u8 *p;

    p = map + y * map_file.w + x;
    return p;
}
/* map_get -- 02ab:0112, 28 bytes */
u8 map_get(i16 x, i16 y)
{
    u8 *p;

    p = map + y * map_file.w + x;
    return *p;
}
/* feature_loc -- 02ab:012e, 24 bytes */
u8 *feature = 0;           /* DS:0x04ac, plane 1: features */

u8 *feature_loc(i16 x, i16 y)
{
    u8 *p;

    p = feature + y * map_file.w + x;
    return p;
}
/* feature_get -- 02ab:0146, 28 bytes */
u8 feature_get(i16 x, i16 y)
{
    u8 *p;

    p = feature + y * map_file.w + x;
    return *p;
}
/* feature_set -- 02ab:0162, 54 bytes */
void feature_set(i16 x, i16 y, i16 mask, i16 on)
{
    u8 *p;

    p = feature_loc(x, y);
    if (on)
        *p |= mask;
    else
        *p &= ~mask;
}
/* continent_loc -- 02ab:0198, 24 bytes */
u8 *continent = 0;         /* DS:0x04b0, plane 2: continent | owner << 4 */
u8 *site = 0;              /* DS:0x04b4, plane 3: sight bits */
void *terrain_1 = 0;                  /* DS:0x04b8, the tile sheet */
void *terrain_2 = 0;                  /* DS:0x04bc */
void *tiles = 0;                      /* DS:0x04c0, the icon sheet */
void *tiles2 = 0;                     /* DS:0x04c4 */
i16 map_center_x = 0;                     /* DS:0x04c8 */
i16 map_center_y = 0;                     /* DS:0x04ca */
i32 map_memory = 0;                      /* DS:0x04cc */
i16 map_scale = 0;                        /* DS:0x04d0 */
i16 map_sprite_scale = 100;               /* DS:0x04d2 */
i16 map_scroll_speed = 10;                /* DS:0x04d4 */
i16 map_colony_mode = 0;                  /* DS:0x04d6 */
i16 map_force = 0;                        /* DS:0x04d8 */
i16 map_view_base_terrain = 0;            /* DS:0x04da */
i16 map_base_seed = 0;                    /* DS:0x04dc */
i16 resource_index[TERRAIN_COUNT] = {                /* DS:0x04de: the resource per terrain (-1: none) */
    6,
    1, 2, 3, 4, 5, 6, 6, 9,
    1, 8, 9, 10, 10, 6, 6, 9,
    1, 8, 9, 10, 10, 6, 6, -1,
    7, -1, 12, 13
};
/* 2 unused declarations restore the count the definitions above moved (mod 17). */
extern i16 pad_d2_0;
extern i16 pad_d2_1;

u8 *continent_loc(i16 x, i16 y)
{
    u8 *p;

    p = continent + y * map_file.w + x;
    return p;
}
/* continent_get -- 02ab:01b0, 30 bytes */
u8 continent_get(i16 x, i16 y)
{
    u8 *p;

    p = continent + y * map_file.w + x;
    return *p;
}
/* continent_at -- 02ab:01ce, 18 bytes */
u8 continent_at(i16 x, i16 y)
{
    return continent_get(x, y) & 0xf;
}
/* continent_set -- 02ab:01e0, 36 bytes */
/* Sets the continent nibble (DOS-only: Windows writes plane 2 elsewhere). */
void continent_set(i16 x, i16 y, i16 c)
{
    u8 *p;

    p = continent_loc(x, y);
    *p ^= (*p ^ c) & 0xf;
}
/* owner_of -- 02ab:0204, 40 bytes */
char owner_of(i16 x, i16 y)
{
    i16 v;

    v = continent_get(x, y) >> 4;
    if (v == 0xf)
        v = -1;
    return v;
}
/* owner_set -- 02ab:022c, 120 bytes */
extern i16 is_village(i16 x, i16 y);
extern void danger(char *, i16, i16, i16);
extern void perform_save(i16);
extern void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2);

/* DOS: a European owner on a village square is reported and stops the game (Windows' check
 * has an empty body). */
void owner_set(i16 x, i16 y, i16 n)
{
    u8 *p;

    if (n < 4 && is_village(x, y) >= 0) {
        danger("Illegal entry into village", n, x, y);
        perform_save(5);
        error_report(-0x54, 1, 0x2d, (i32)x, (i32)y);
    }
    p = continent_loc(x, y);
    *p = (*p & 0xf) | (n << 4);
}
/* land_region_at -- 02ab:02a4, 64 bytes */
extern i16 terrain_is_ocean(i16 x, i16 y);   /* 03e4:0074 */

i16 land_region_at(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (on_map(x, y) && !terrain_is_ocean(x, y))
        n = continent_at(x, y);
    return n;
}
/* site_loc -- 02ab:02e4, 24 bytes */
/* 4 unused declarations set site_loc's operand order (mod 17). */
extern i16 pad_site_loc_0, pad_site_loc_1, pad_site_loc_2, pad_site_loc_3;
u8 *site_loc(i16 x, i16 y)
{
    u8 *p;

    p = site + y * map_file.w + x;
    return p;
}
/* site_get -- 02ab:02fc, 28 bytes */
u8 site_get(i16 x, i16 y)
{
    u8 *p;

    p = site + y * map_file.w + x;
    return *p;
}
/* is_unit -- 02ab:0318, 46 bytes */
/* The owner of a unit on the square, or -1. */
i16 is_unit(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_UNIT)
        n = owner_of(x, y);
    return n;
}
/* is_colony -- 02ab:0346, 58 bytes */
/* DOS: no on_map test here. */
i16 is_colony(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_SETTLEMENT) {
        n = owner_of(x, y);
        if (n >= 4)
            n = -1;
    }
    return n;
}
/* is_village -- 02ab:0380, 58 bytes */
/* A settlement whose owner is a tribe (4 and up): the tribe, or -1. */
i16 is_village(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_SETTLEMENT) {
        n = owner_of(x, y);
        if (n < 4)
            n = -1;
    }
    return n;
}
/* is_city -- 02ab:03ba, 46 bytes */
/* Any settlement: its owner, or -1. */
i16 is_city(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_SETTLEMENT)
        n = owner_of(x, y);
    return n;
}
/* is_anything -- 02ab:03e8, 34 bytes */
i16 is_anything(i16 x, i16 y)
{
    i16 n;

    n = is_city(x, y);
    if (n < 0)
        n = is_unit(x, y);
    return n;
}
/* is_hostile -- 02ab:040a, 78 bytes */
extern Nation europe[];                   /* DS:0x4e80 */

/* The European owner of a road/plowed square that `me` is at war with (rel bit 0x40), or -1.
 * Windows is_hostile without its map_index_in_range test. */
i16 is_hostile(i16 x, i16 y, i16 me)
{
    i16 rl;
    i16 n;

    rl = -1;
    if (feature_get(x, y) & (MAP1_ROAD | MAP1_PLOWED)) {
        n = owner_of(x, y);
        if (n >= 0 && n < 4 && n != me && (europe[me].rel[n] & 0x40))
            rl = n;
    }
    return rl;
}
/* resource_at -- 02ab:0458, 232 bytes */
extern i16 terrain_at(i16 x, i16 y);   /* 03e4:003a */

/* DOS: the hash is (x>>2) + (y>>2)*3 - forested + seed (Windows: *17, *19, *31). */
i16 resource_at(i16 x, i16 y)
{
    i16 r;
    i16 nhash;
    i16 cls;
    i16 terrain;
    i16 h;

    r = -1;
    if (map_base_seed == 0)
        goto done;
    if (is_village(x, y) >= 0)
        goto done;
    terrain = map_get(x, y);
    terrain &= 0xff3f;
    if ((terrain >= 8 && terrain < 0x10) || (terrain >= 0x10 && terrain < 0x18))
        cls = 1;
    else
        cls = 0;
    nhash = ((y >> 2) * 3 + (x >> 2) - cls + map_base_seed) & 0xf;
    h = ((x & 3) << 2) + (y & 3);
    if (nhash != h && (nhash ^ 0xa) != h)
        goto done;
    r = resource_index[terrain_at(x, y)];
    if (r == 0)
        r = 6;
    if (feature_get(x, y) & MAP1_DEPLETED) {
        if (r == 12)
            r = 0;
        else
            r = -1;
    }
done:
    return r;
}
/* lost_city_at -- 02ab:0540, 124 bytes */
i16 lost_city_at(i16 x, i16 y)
{
    i16 r;
    i16 his;
    i16 hash;
    i16 terrain;

    r = 0;
    if (map_base_seed == 0)
        goto done;
    terrain = terrain_at(x, y);
    if (terrain == TERRAIN_OCEAN || terrain == TERRAIN_SEA_LANE)
        goto done;
    if (terrain == TERRAIN_ARCTIC)
        goto done;
    his = owner_of(x, y);
    if (his >= 0)
        goto done;
    hash = (y >> 2) * 19 + (x >> 2) * 17 + map_base_seed + 8;
    if (((x & 3) << 2) + (y & 3) != (hash & 0x1f))
        goto done;
    r = 1;
done:
    return r;
}
/* terrain_fix_2 -- 02ab:05bc, 74 bytes */
/* The terrain a square shows in the current map frame: in frame 2 both forest bands draw as the
 * first band, in frame 3 every forest as its open terrain. */
u8 terrain_fix_2(i16 t)
{
    t = (u8)t & 0x1f;
    switch (map_view_base_terrain) {
    case 2:
        if (t < 0x18 && t >= 8)
            t = t & 7 | 8;
        break;
    case 3:
        if (t < 0x18)
            t &= 7;
        break;
    }
    return t;
}

/* The map module's uninitialised data: communals (int x;), placed by the linker (BSS, from
 * DS:0x4999). A definition after the last function moves no symbol position. */
i16 map_corner_x;                         /* DS:0x49f2 */
i16 map_corner_y;                         /* DS:0x49f4 */
char map_name[80];                        /* DS:0x4a18 */
DosPort scr_feature;                      /* DS:0x4a6c */
MapSize map_file;                         /* DS:0x4b12 */
i16 map_area_size_x;                      /* DS:0x4e70 */
i16 map_area_size_y;                      /* DS:0x4e76 */
i16 map_tile_size_x;                      /* DS:0x4e8a */
i16 map_tile_size_y;                      /* DS:0x4e8c */
DosPort scr_map;                          /* DS:0x4e92 */
i32 map_size;                            /* DS:0x4e9c */
char sea_moves[270];                      /* DS:0x50ba: 270 bytes, nothing in the editor reads it */
i16 region_size[16];                      /* DS:0x5270 */
i16 map_view_size_x;                      /* DS:0x52d4 */
i16 map_view_size_y;                      /* DS:0x52d8 */
DosPort scr_site;                         /* DS:0x5aa2 */
i16 map_tile_inset_x;                     /* DS:0x5ad8 */
i16 map_tile_inset_y;                     /* DS:0x5af4 */
i16 map_area_base_x;                      /* DS:0x5af6 */
i16 map_area_base_y;                      /* DS:0x5af8 */
DosPort scr_continent;                    /* DS:0x5eba */
char land_moves[270];                     /* DS:0x5f18: 270 bytes, nothing in the editor reads it */
i16 map_corner_x2;                        /* DS:0x6026 */
i16 map_corner_y2;                        /* DS:0x603e */
i16 map_pixel_size_x;                     /* DS:0x64ec */
i16 map_pixel_size_y;                     /* DS:0x64ee */

