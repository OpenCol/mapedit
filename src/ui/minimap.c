/* minimap.c -- the minimap: the map at a pixel a square, and the view's rectangle on it. */
/*
 * The minimap sits at the top right of the screen: 56 x 39 squares, one pixel each, drawn
 * into the work screen with its top-left square at pixel (0xfc, 9), inside a frame and on the
 * backdrop tile. It shows the squares from mini_x, mini_y, which follow the view's centre
 * and stay inside the map's border; a white rectangle marks the squares the map view shows.
 * Each square is the colour terrain_colors gives its terrain, a table built at startup from
 * the centre pixel of each terrain's tile or icon.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "assets.h"
#include "font.h"
#include "menu.h"
#include "popup.h"
#include "buffer.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mouse.h"
#include "screen.h"
#include "sprite.h"

#include "units.h"
#include "terrain.h"
#include "records.h"

/* The colour at the centre (8, 8) of terrain icon `icon`, drawn into a 16 x 16 scratch
 * buffer. The buffer is not cleared first: were the icon's centre pixel transparent, the
 * result would be whatever the buffer held. */
u8 get_a_color(i16 icon)
{
    u8 buf[256];
    DosPort port;

    port.w = port.h = 16;
    port.bits = buf;
    sprite_draw(icon, tiles, &port, 0, 0);
    return buf[0x88];
}
/* The colour at the centre (8, 8) of terrain tile `tile`. */
u8 get_a_color_2(i16 tile)
{
    u8 buf[256];
    DosPort port;

    port.w = port.h = 16;
    port.bits = buf;
    tile_draw(terrain_1, tile, &port, 0, 0);
    return buf[0x88];
}
/* Fills terrain_colors, the minimap's colour for each terrain id: the eight open terrains
 * from their tiles, each forest the colour of its open terrain (so forests do not show on the
 * minimap), arctic, ocean and sea lane from their tiles, and mountains and hills from their
 * single-square icons. */
void get_tile_colors(void)
{
    i16 i;

    for (i = 0; i < 8; i++) {
        terrain_colors[i] = get_a_color_2(i);
        terrain_colors[i + 16] = get_a_color_2(i);
        terrain_colors[i + 8] = get_a_color_2(i);
    }
    terrain_colors[24] = get_a_color_2(0x18);
    terrain_colors[25] = get_a_color_2(0x19);
    terrain_colors[26] = get_a_color_2(0x1a);
    terrain_colors[TERRAIN_MOUNTAINS] = get_a_color(0x21);
    terrain_colors[TERRAIN_HILLS] = get_a_color(0x31);
}
/* Places the minimap: its top-left square 28 columns left of and 19 rows above the view's
 * centre, kept inside the map's border. */
void generate_mini(void)
{
    mini_x = map_center_x - 28;
    mini_y = map_center_y - 19;
    mini_x = minimax(mini_x, 1, map_file.w - 57);
    mini_y = minimax(mini_y, 1, map_file.h - 40);
}
/* Draws the squares (x, y) to (x + cols - 1, y + rows - 1) into the minimap in the work
 * screen, one pixel each: white (15) when `white` is set; black where nation `viewer` has not
 * seen the square (its bit in the site plane's high nibble), unless viewer is -1, which sees
 * everything; otherwise the colour of the square's terrain, hills and mountains taking theirs
 * whatever lies under them. The region must lie inside the minimap. The feature and
 * continent planes are walked alongside, but nothing uses what is read from them. */
void blast_mini_region(i16 x, i16 y, i16 cols, i16 rows, i16 viewer, i16 white)
{
    u8 *p0;
    u8 *p3;
    u8 *dst;
    u8 terrain;
    u8 colour;
    u8 site;
    i16 mapw;
    i16 portw;
    i16 col;
    i16 row;
    i16 mask;

    if (viewer >= 0)
        mask = 0x10 << viewer;
    else
        mask = 0;
    p0 = map_loc(x, y);
    p3 = site_loc(x, y);
    dst = buffer_pointer(&scr_work, x - mini_x + 0xfc, y - mini_y + 9);
    mapw = map_file.w;
    portw = scr_work.w;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            terrain = *p0++;
            site = *p3++;
            if (white) {
                colour = 15;
            } else if (mask && !(mask & site)) {
                colour = 0;
            } else {
                if (terrain & MAP0_HILLY) {
                    if (terrain & MAP0_MOUNTAINS)
                        terrain = TERRAIN_MOUNTAINS;
                    else
                        terrain = TERRAIN_HILLS;
                } else {
                    terrain = terrain & MAP0_TERRAIN_MASK;
                }
                colour = terrain_colors[terrain];
            }
            *dst++ = colour;
        }
        p0 += mapw - cols;
        p3 += mapw - cols;
        dst += portw - cols;
    }
}
/* Draws every square of the minimap as `viewer` sees it. */
void blast_mini(i16 viewer)
{
    blast_mini_region(mini_x, mini_y, 0x38, 0x27, viewer, 0);
}
/* Redraws the whole minimap: re-places it, fills its corner of the screen with the backdrop
 * tile (or black without one), draws its frame, the squares as `viewer` sees them and the
 * view's rectangle in white; with `update` set, copies the corner to the screen. */
void show_mini(i16 update, i16 viewer)
{
    generate_mini();
    if (main_tile == 0)
        buffer_rect_fill(0xf1, 8, 0x4f, scr_work, 0x29, 0);
    else
        buffer_tile(scr_work, *main_tile, 0xf1, 8, 0x4f, 0x29, 0, 0);
    buffer_draw_box(0xfb, 8, 0x134, scr_work, 0x30, 6);
    blast_mini(viewer);
    buffer_draw_box(max(mini_x, map_corner_x) - mini_x + 0xfc, max(mini_y, map_corner_y) - mini_y + 9,
               min(mini_x + 55, map_corner_x2) - mini_x + 0xfc, scr_work,
               min(mini_y + 38, map_corner_y2) - mini_y + 9, 15);
    if (update)
        mouse_video_update(0xf1, 8, 0xf1, 8, 0x4f, 0x29);
}
/* Re-places the minimap for a new or newly loaded map. */
void new_mini(void)
{
    generate_mini();
}

i16 mini_y;                               /* the map row of the minimap's top row */
u8 terrain_colors[30];         /* the minimap colour of each terrain id (get_tile_colors) */
i16 mini_x;                               /* the map column of the minimap's left column */

