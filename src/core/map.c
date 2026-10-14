/* map.c -- the map's four planes, reading and writing a square, and what stands on it.
 *
 * The map is four byte arrays of map_file.w x map_file.h squares, each indexed
 * y * map_file.w + x:
 *
 *   map        terrain: the low five bits are the terrain type (TERRAIN_*), unless
 *              MAP0_HILLY marks hills or mountains; MAP0_RIVER and MAP0_MAJOR_RIVER
 *              add a river (terrain.h has the bits)
 *   feature    MAP1_* bits: a unit, a settlement, a depleted resource, a road, ...
 *   continent  the low nibble is the region number Find Continents gives (land and sea
 *              are numbered separately), the high nibble the owner (0xf: nobody)
 *   site       the sight bits: 0x10 << nation where that nation has seen the square
 *
 * The first three are what a .MP file holds (mapfile.c); the editor neither loads nor
 * saves the site plane. The outermost ring of squares is a border: on_map excludes it, and
 * terrain_at reports it as ocean.
 *
 * Most of this module is shared with the game, so it also answers questions the editor
 * never asks: owners, settlements, units, resources and Lost City Rumours. The editor never
 * sets map_base_seed, so resource_at and lost_city_at always report nothing; the settlement
 * and unit tests read MAP1_* bits that only a game would set.
 *
 * The rest is the map view's geometry (the visible window, the scale, the tile size), which
 * mapview.c computes and the editor's commands read.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "game.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "error.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "units.h"

#include "terrain.h"
#include "records.h"

#define ABS(x) ((x) > 0 ? (x) : ~(x) + 1)


/* Is (x, y) a playable square? 0 on the border ring and beyond, 1 inside it. */
i16 on_map(i16 x, i16 y)
{
    i16 r;

    r = 1;
    if (x < 1 || y < 1 || map_file.w - 1 <= x || map_file.h - 1 <= y)
        r = 0;
    return r;
}

/* Is the offset (dx, dy) from a colony within the reach that mode gives? Mode 1 takes the
 * square itself and its four orthogonal neighbours, mode 2 adds the diagonals, mode 3 adds
 * the squares two away in a straight line. Any other mode adds every square with |dx| < 2 or
 * |dy| < 2, with no limit on the other axis: callers pass offsets within two squares, where
 * that means the 5x5 block without its corners. Only the colony view (map_colony_mode) uses
 * it, and the editor never enters that view. */
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

/* Is (x, y) inside the visible map window (map_corner_x..x2, map_corner_y..y2 inclusive)? */
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

i16 map_file_version = 0;   /* the version word of the loaded map file (4) */
char map_ext[4] = "mp";     /* the map files' extension */
i16 map_error = 0;          /* why the last map file operation failed (mapfile.c), or 0 */
/* Set by map_check for a map of more than 12000 squares, which the editor refuses. The map
 * view keeps the game's branch for such a map (the planes then hold only the area around the
 * view, map_area_*), but after any successful load or new map this is 0. */
i16 map_high_memory_mode = 0;
u8 *map = 0;                /* the terrain plane */

/* The address of a square's byte in each plane (map_loc, feature_loc, continent_loc,
 * site_loc), and its value (map_get, ...). None of them checks the coordinates. */
u8 *map_loc(i16 x, i16 y)
{
    u8 *p;

    p = map + y * map_file.w + x;
    return p;
}

u8 map_get(i16 x, i16 y)
{
    u8 *p;

    p = map + y * map_file.w + x;
    return *p;
}

u8 *feature = 0;            /* the feature plane: MAP1_* bits */

u8 *feature_loc(i16 x, i16 y)
{
    u8 *p;

    p = feature + y * map_file.w + x;
    return p;
}

u8 feature_get(i16 x, i16 y)
{
    u8 *p;

    p = feature + y * map_file.w + x;
    return *p;
}

/* Sets (on != 0) or clears the bits of mask in a square's features. */
void feature_set(i16 x, i16 y, i16 mask, i16 on)
{
    u8 *p;

    p = feature_loc(x, y);
    if (on)
        *p |= mask;
    else
        *p &= ~mask;
}

u8 *continent = 0;          /* the continent plane: region number | owner << 4 */
u8 *site = 0;               /* the sight plane: 0x10 << nation where that nation has seen */
void *terrain_1 = 0;        /* the terrain tile sheet */
void *terrain_2 = 0;        /* unused */
void *tiles = 0;            /* the map icon sheet (PHYS0): hills, rivers, resources, ... */
void *tiles2 = 0;           /* the same sheet as tiles once it has loaded */
i16 map_center_x = 0;       /* the square the view is centred on */
i16 map_center_y = 0;
i32 map_memory = 0;         /* the bytes allocated for each plane */
i16 map_scale = 0;          /* the view's zoom level: 0 (16-pixel squares) to 3 (2-pixel) */
i16 map_sprite_scale = 100; /* the percentage icons are drawn at, from map_scale */
i16 map_scroll_speed = 10;  /* set from map_scale; the editor does not read it */
i16 map_colony_mode = 0;    /* nonzero: the game's 5x5 colony view; always 0 in the editor */
i16 map_force = 0;          /* nonzero: map_startup skips the file's header (always, here) */
/* Which terrain the view draws (terrain_fix_2): 0 as stored, 2 one forest band, 3 no forests.
 * Always 0 in the editor. */
i16 map_view_base_terrain = 0;
/* The seed that places resources and Lost City Rumours; 0 means none. The editor never sets
 * it. */
i16 map_base_seed = 0;
/* The resource each terrain type can carry (resource_at), as an offset into the map icons;
 * -1: none. */
i16 resource_index[TERRAIN_COUNT] = {
    6,
    1, 2, 3, 4, 5, 6, 6, 9,
    1, 8, 9, 10, 10, 6, 6, 9,
    1, 8, 9, 10, 10, 6, 6, -1,
    7, -1, 12, 13
};
u8 *continent_loc(i16 x, i16 y)
{
    u8 *p;

    p = continent + y * map_file.w + x;
    return p;
}

u8 continent_get(i16 x, i16 y)
{
    u8 *p;

    p = continent + y * map_file.w + x;
    return *p;
}

/* The region number of a square: the low nibble of its continent byte. */
u8 continent_at(i16 x, i16 y)
{
    return continent_get(x, y) & 0xf;
}

/* Sets a square's region number (the low nibble), keeping its owner. */
void continent_set(i16 x, i16 y, i16 c)
{
    u8 *p;

    p = continent_loc(x, y);
    *p ^= (*p ^ c) & 0xf;
}

/* The owner of a square (the high nibble of its continent byte), or -1 for nobody (0xf). */
char owner_of(i16 x, i16 y)
{
    i16 v;

    v = continent_get(x, y) >> 4;
    if (v == 0xf)
        v = -1;
    return v;
}

/* Sets a square's owner to n (-1: nobody), keeping its region number. Giving a village's square
 * to a European nation, or to nobody (-1 is below 4 too), is a fatal error: the game's danger
 * and perform_save hooks run (empty in the editor), then error_report shuts the program down.
 * Only map_init calls this, and nothing calls map_init. */
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

/* The region number of a land square inside the border, or -1 for water and the border. */
i16 land_region_at(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (on_map(x, y) && !terrain_is_ocean(x, y))
        n = continent_at(x, y);
    return n;
}

u8 *site_loc(i16 x, i16 y)
{
    u8 *p;

    p = site + y * map_file.w + x;
    return p;
}

u8 site_get(i16 x, i16 y)
{
    u8 *p;

    p = site + y * map_file.w + x;
    return *p;
}

/* The owner of a unit on the square (MAP1_UNIT), or -1. */
i16 is_unit(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_UNIT)
        n = owner_of(x, y);
    return n;
}

/* The European owner (0-3) of a settlement on the square, or -1. */
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

/* Any settlement: its owner, or -1. */
i16 is_city(i16 x, i16 y)
{
    i16 n;

    n = -1;
    if (feature_get(x, y) & MAP1_SETTLEMENT)
        n = owner_of(x, y);
    return n;
}

/* The owner of a settlement on the square, else of a unit, else -1. */
i16 is_anything(i16 x, i16 y)
{
    i16 n;

    n = is_city(x, y);
    if (n < 0)
        n = is_unit(x, y);
    return n;
}

/* The resource on a square (an offset into the map icons), or -1. Resources are not stored:
 * they follow from map_base_seed. Each 4x4 block of squares has a number,
 * (y/4)*3 + x/4 + seed, less 1 on a forest square (types 8-23; hills and mountains count
 * as open), taken mod 16; the two squares of the block whose position ((x%4)*4 + y%4) equals
 * that number, or that number xor 10, carry the resource resource_index gives their terrain.
 * A 0 from the table would read as 6 (the table has none). A village square has none. On a
 * depleted square (MAP1_DEPLETED) resource 12 becomes 0 and any other is gone. Always -1 in
 * the editor, where the seed is 0. */
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

/* Is there a Lost City Rumour on the square? Like resources, rumours follow from
 * map_base_seed: at most one per 4x4 block, at the position ((x%4)*4 + y%4) equal to
 * ((y/4)*19 + (x/4)*17 + seed + 8) mod 32. Positions only run to 15, so about half the blocks
 * get none. Never on water, arctic or an owned square, and never in the editor (seed 0). */
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

/* The terrain type t (the low five bits of a terrain byte) as the view draws it under
 * map_view_base_terrain: 2 draws both forest bands as the first (8-15), 3 draws every forest
 * as its open terrain (0-7), anything else leaves t alone. */
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

/* The view's geometry is compute_view_parameters' (mapview.c): the visible window runs from
 * map_corner_x, map_corner_y to map_corner_x2, map_corner_y2 (inclusive). */
i16 map_corner_x;
i16 map_corner_y;
char map_name[80];          /* the map file's name, given an .MP extension when it is opened */
DosPort scr_feature;        /* the feature plane as a w x h port, for buffer_get and buffer_put */
MapSize map_file;           /* the map's width and height in squares: a .MP file's header */
/* The part of the map the planes hold: the whole map (map_area_size_x, _y are its width and
 * height), unless map_high_memory_mode. Strides through the planes use map_area_size_x. */
i16 map_area_size_x;
i16 map_area_size_y;
i16 map_tile_size_x;        /* the pixels a square takes in the view (16 >> map_scale) */
i16 map_tile_size_y;
DosPort scr_map;            /* the terrain plane as a port (set_plane_ports) */
i32 map_size;               /* map_file.w * map_file.h, the squares in each plane */
char sea_moves[270];        /* unused in the editor */
i16 region_size[16];        /* the squares in each land region (1-15) Find Continents numbered */
/* The squares the view shows across and down: (15 x 12) << map_scale, less for a map smaller
 * than that. */
i16 map_view_size_x;
i16 map_view_size_y;
DosPort scr_site;           /* the sight plane as a port */
/* The view's margin in squares when the map is narrower or shorter than the view, which
 * centres it. */
i16 map_tile_inset_x;
i16 map_tile_inset_y;
i16 map_area_base_x;        /* the first column and row the planes hold, in that mode */
i16 map_area_base_y;
DosPort scr_continent;      /* the continent plane as a port */
char land_moves[270];       /* unused in the editor */
i16 map_corner_x2;
i16 map_corner_y2;
i16 map_pixel_size_x;       /* the view's size in pixels (240 x 192) */
i16 map_pixel_size_y;

