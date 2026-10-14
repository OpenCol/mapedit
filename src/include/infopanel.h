/* infopanel.h -- the information window beside the map.
 *
 * Defined in ui/infopanel.c. The window sits below the minimap and shows the map's size, the
 * cursor's position, the terrain under it, the selection with the shift-click legend, the
 * brush size and whether coast protection is on. info_window_draw redraws all of it and
 * copies it to the screen; info_terrain and info_terrain_2 write a terrain's or a feature
 * word's name and move down a line.
 */
#ifndef MAPEDIT_INFOPANEL_H
#define MAPEDIT_INFOPANEL_H

#include "msc.h"
void info_window_clear(void);
void info_window_update(void);
void info_terrain(i16 t, i16 *x, i16 *y);
void info_terrain_2(i16 n, i16 *x, i16 *y);
void info_window_draw(void);

#endif
