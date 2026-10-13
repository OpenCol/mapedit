/* assets.h */
#ifndef MAPEDIT_ASSETS_H
#define MAPEDIT_ASSETS_H

#include "msc.h"
extern void *cursor;
extern void *icons;
extern void *buildings;
extern DosPort *main_tile;
extern DosPort scr_back;
extern DosPort scr_slide;
void *load_terrain_series(char *name);
i16 load_terrain_tiles(void);

#endif
