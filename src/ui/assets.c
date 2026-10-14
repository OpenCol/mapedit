/* assets.c -- the art the editor loads: its sprite series, the terrain tiles, the backdrop tile.
 *
 * The sprite series are MADSPACK files from the game folder, loaded by the editor's main
 * (mapedit.c), which also fills scr_back with the wood backdrop and points main_tile at it.
 * This file owns those globals and turns the terrain series into the 16 x 16 tile sheet the
 * map view, the minimap, the info panel and Map Tile Select draw from (terrain_1).
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "font.h"
#include "assets.h"
#include "error.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"

void *cursor = 0;                           /* CURSOR's sprite series (loaded, never drawn) */

void *icons = 0;                            /* ICONS (the map's cursor square, by zoom) */

void *buildings = 0;                        /* unused */

DosPort *main_tile = 0;                         /* the tile the backgrounds are filled with */

DosPort scr_back;                         /* the 32 x 24 wood backdrop tile (WOODTILE) */

DosPort scr_slide;                        /* unused */

/* Loads the sprite series `name` and draws its first 12 sprites into a new sheet of 12
 * 16 x 16 tiles, sprite i + 1 into tile i. Returns the sheet, or 0 with exit_value set:
 * 0x321 when the sheet cannot be allocated, 0x322 when the series cannot be loaded (the
 * sheet allocated before is not freed then). The series itself is freed. */
void *load_terrain_series(char *name)
{
    DosPort port;
    char *tiles;
    void *sheet;
    void *r;
    i16 n;
    i16 i;

    r = 0;
    n = 12;
    tiles = tile_allocate(n);
    if (tiles == 0) {
        exit_value = 0x321;
        goto out;
    }
    sheet = sprite_series_load(name, 0x4000);
    if (sheet == 0) {
        exit_value = 0x322;
        goto out;
    }
    port.h = port.w = 16;
    for (i = 0; i < n; i++) {
        port.bits = tiles + i * 0x100;
        sprite_draw(i + 1, sheet, &port, 0, 0);
    }
    mem_free(sheet);
    r = tiles;
out:
    return r;
}

/* Loads the terrain tiles (TERRAIN) into terrain_1. Returns 0 on success, 1 on failure. */
i16 load_terrain_tiles(void)
{
    i16 r;

    r = 1;
    if ((terrain_1 = load_terrain_series("terrain")) != 0)
        r = 0;
    return r;
}
