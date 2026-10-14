/* assets.h -- the art the editor loads: its sprite series, the terrain tiles, the backdrop.
 *
 * Defined in ui/assets.c. The editor's main (editor/mapedit.c) loads the sprite series and
 * the wood backdrop at startup; load_terrain_series draws a terrain series into a sheet of
 * 16 x 16 tiles, the form the map view, the minimap, the info panel and Map Tile Select draw
 * terrain from.
 */
#ifndef MAPEDIT_ASSETS_H
#define MAPEDIT_ASSETS_H

#include "msc.h"
extern void *cursor;            /* CURSOR's sprite series (loaded, never drawn) */
extern void *icons;             /* ICONS: the map's cursor square at each zoom */
extern void *buildings;         /* unused */
extern DosPort *main_tile;      /* the tile backgrounds are filled with */
extern DosPort scr_back;        /* the 32 x 24 wood backdrop tile (WOODTILE) */
extern DosPort scr_slide;       /* unused */
void *load_terrain_series(char *name);
i16 load_terrain_tiles(void);

#endif
