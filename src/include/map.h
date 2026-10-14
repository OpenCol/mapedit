/* map.h -- the map: its planes and squares, its files, the terrain table, and drawing it.
 *
 * The map is four planes of map_file.w x map_file.h bytes, one byte a square, indexed
 * y * map_file.w + x (terrain.h has the bits):
 *   map        the terrain: a terrain id, or hills or mountains, and a river;
 *   feature    the features (MAP1_*): units, settlements, roads, plowing;
 *   continent  the region number in the low nibble, the owner in the high one;
 *   site       the sight bits, one per European nation (not saved).
 * The planes are allocated once, at startup, 12000 bytes each, and every map the editor opens
 * or creates is held in those blocks. A .MP file holds the map's width and height, version 4
 * and the first three planes.
 *
 * Defined in several files across the layers, which share these declarations:
 *   core/map.c          the planes and reading or writing a square; what stands on it
 *   core/mapfile.c      allocating the planes, loading and saving .MP files, a blank map
 *   core/terrain.c      the terrain table from NAMES.TXT; the terrain of a square
 *   core/continents.c   Find Continents
 *   core/compass.c      the eight directions as x and y steps
 *   ui/mapview.c        the view's geometry; drawing the terrain, coasts, rivers and seams
 *   ui/minimap.c        the minimap
 *   ui/tile.c           16 x 16 terrain tiles
 *   ui/write.c          adding a space or a stored word to a string, and drawing text
 */
#ifndef MAPEDIT_MAP_H
#define MAPEDIT_MAP_H

#include "msc.h"

#pragma pack(push, 2)
/* A terrain type: one row of NAMES.TXT (load_terrain), filling terrain[] in id order. name is
 * its word in the string store; f2, f3 and f4 take the row's next three numbers (movement,
 * defence, improvement turns) and v[] the nine after those (the value and the first eight
 * yields: the ninth yield, the fisherman's, is not read). pad is never written. The editor
 * uses only the name. */
typedef struct { i16 name; char f2; char f3; char f4; char pad[2]; char v[9]; } TerrainInfo;
#pragma pack(pop)
#include "records.h"
#include "terrain.h"

/* ui/tile.c */
void tile_draw(char *tiles, i16 t, DosPort *port, i16 x, i16 y);
void tile_draw_mask(char *tiles, i16 t, DosPort *port, i16 x, i16 y);
void tile_draw_scale(char *tiles, i16 t, DosPort *port, i16 x, i16 y, i16 shift);
void tile_draw_scale_mask(char *tiles, i16 t, DosPort *port, i16 x, i16 y, i16 shift);
/* core/map.c: a square's byte in each plane, and what stands on it */
u8 continent_get(i16 x, i16 y);
u8 feature_get(i16 x, i16 y);
u8 *feature_loc(i16 x, i16 y);
i16 is_village(i16 x, i16 y);
i16 lost_city_at(i16 x, i16 y);
u8 map_get(i16 x, i16 y);
u8 *map_loc(i16 x, i16 y);
i16 on_colony_map(i16 dx, i16 dy, i16 mode);
i16 on_map(i16 x, i16 y);
char owner_of(i16 x, i16 y);
i16 resource_at(i16 x, i16 y);
u8 *site_loc(i16 x, i16 y);
u8 terrain_fix_2(i16 t);
/* ui/mapview.c: the view and drawing the map */
void conform_to_view_size(i16 *x, i16 *y, i16 *w, i16 *h);
void show_map(void);
void show_map_region(i16 x, i16 y, i16 w, i16 h);
void update_terrain(void);
void update_terrain_region(i16 x, i16 y, i16 w, i16 h);
void draw_map_2(void);
void draw_map_region_2(i16 x, i16 y, i16 w, i16 h);
i16 map_find_continents(void);         /* core/continents.c */
/* core/mapfile.c */
i16 allocate_map_memory(i16 big);
i16 create_blank_map(i16 w, i16 h);
i16 initialize_map_memory(void);
i16 load_map_file(void);
i16 map_check(void);
i16 map_startup(i16 big);
void set_plane_ports(void);
i16 write_map_file(void);
/* ui/mapview.c: the terrain renderer */
i16 coast_scan(void);
void compute_view_parameters(void);
void draw_map_icon(i16 icon);
void draw_map_tile(i16 tile);
void draw_map_tile2(i16 tile);
void draw_seams(i16 hidden_only, i16 no_scan, i16 always);
void draw_square(i16 hidden);
void generate_terrain_map(i16 nation);
void generate_terrain_map_region(i16 x, i16 y, i16 w, i16 h, i16 nation);
i16 is_land_seam(i16 unused, i16 off);
i16 nb8_mask_p1(i16 mask, i16 maxzoom);
i16 nb_land_seam(i16 a, i16 maxzoom);
i16 nb_mask_p0(i16 mask, i16 maxzoom);
i16 nb_match_p0(i16 val, i16 maxzoom);
/* ui/minimap.c */
void blast_mini(i16 viewer);
void blast_mini_region(i16 x, i16 y, i16 cols, i16 rows, i16 viewer, i16 white);
void generate_mini(void);
u8 get_a_color(i16 icon);
u8 get_a_color_2(i16 tile);
void get_tile_colors(void);
void new_mini(void);
void show_mini(i16 update, i16 viewer);
/* core/terrain.c */
i16 terrain_at(i16 x, i16 y);
i16 terrain_is_ocean(i16 x, i16 y);
i16 terrain_type(i16 b);
/* ui/tile.c */
char *tile_allocate(i16 n);
i16 tile_id(i16 t);
/* ui/write.c: strings built from words; text drawn on the work screen */
void say_string(char *buf, i16 word);
void space_string(char *buf);
i16 write_right(char *s, i16 x, i16 y, i16 color);
i16 write_string(char *s, i16 x, i16 y);

/* x and y steps to the neighbours: 4 orthogonal, 8 around (core/compass.c) */
extern i8 card_x[6];
extern i8 card_y[6];
extern i8 move_x[10];
extern i8 move_y[10];
extern u8 *continent;            /* the continent plane: region | owner << 4 */
extern u8 *feature;              /* the feature plane (MAP1_*) */
extern u8 *map;                  /* the terrain plane */
extern i16 map_area_base_x;
extern i16 map_area_base_y;
extern i16 map_area_size_x;
extern i16 map_area_size_y;
extern i16 map_base_seed;
extern i16 map_center_x;         /* the square the view is centred on */
extern i16 map_center_y;
extern i16 map_colony_mode;
extern i16 map_corner_x;         /* the first and last squares the view shows */
extern i16 map_corner_x2;
extern i16 map_corner_y;
extern i16 map_corner_y2;
extern i16 map_error;            /* why the last map file operation failed, or 0 */
extern char map_ext[4];          /* "mp", the map files' extension */
extern MapSize map_file;         /* the map's width and height, in squares */
extern i16 map_file_version;     /* the .MP version: 4 */
extern i16 map_force;
extern i16 map_high_memory_mode;
extern i32 map_memory;           /* the bytes allocated for each plane */
extern char map_name[80];        /* the map file's name */
extern i16 map_pixel_size_x;     /* the view's size in pixels (240 x 192) */
extern i16 map_pixel_size_y;
extern i16 map_scale;            /* the zoom: 0 (16-pixel squares) to 3 (2-pixel) */
extern i16 map_scroll_speed;
extern i32 map_size;             /* the squares in each plane */
extern i16 map_sprite_scale;
extern i16 map_tile_inset_x;
extern i16 map_tile_inset_y;
extern i16 map_tile_size_x;      /* a square's size in pixels at this zoom */
extern i16 map_tile_size_y;
extern i16 map_view_base_terrain;
extern i16 map_view_size_x;      /* the squares the view shows across and down */
extern i16 map_view_size_y;
extern i16 region_size[16];      /* the squares in each land region */
extern i16 resource_index[TERRAIN_COUNT];
/* the planes as ports, so the buffer code can address them by (x, y) */
extern DosPort scr_continent;
extern DosPort scr_feature;
extern DosPort scr_map;
extern DosPort scr_site;
extern u8 *site;                 /* the sight plane: 0x10 << nation where it has seen */
extern void *terrain_1;          /* the terrain tile sheet */
extern void *tiles;              /* the map icon sheet: hills, rivers, resources */
extern void *tiles2;
/* the terrain renderer's current square (ui/mapview.c) */
extern u8 gen_base_tile;
extern u8 gen_feature_tile;
extern u8 gen_map_tile;
extern u8 gen_site_tile;
extern i16 gen_tile_x;
extern i16 gen_tile_y;
extern i16 gen_variation;
extern u8 map_player_mask;
extern u8 *scan_feature;
extern u8 scan_flags;
extern u8 scan_index;
extern u8 *scan_map;
extern u8 *scan_site;
extern i16 scan_x;
extern i16 scan_y;
extern i16 mini_x;               /* the map square at the minimap's top left */
extern i16 mini_y;
extern u8 terrain_colors[30];    /* the minimap's colour for each terrain */
extern TerrainInfo terrain[29];  /* the terrain table, by terrain id */
extern i16 terrain_names[5];     /* Forest, River, Major River, Minor River, Unexplored */
void load_terrain(i16 n);
void load_data(void);
void forest_fix(void);

#endif
