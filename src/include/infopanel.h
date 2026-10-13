/* infopanel.h */
#ifndef MAPEDIT_INFOPANEL_H
#define MAPEDIT_INFOPANEL_H

#include "msc.h"
void info_window_clear(void);
void info_window_update(void);
void info_terrain(i16 t, i16 *x, i16 *y);
void info_terrain_2(i16 n, i16 *x, i16 *y);
void info_window_draw(void);

#endif
