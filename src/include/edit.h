/* edit.h */
#ifndef MAPEDIT_EDIT_H
#define MAPEDIT_EDIT_H

#include "msc.h"
void change_map(i16 x, i16 y, i16 pickup);
void fill_map(i16 x, i16 y, i16 pickup);
void set_center(i16 x, i16 y, i16 flag);
i16 possibly_center(i16 x, i16 y, i16 x2, i16 y2, i16 flag);
void move_cursor(i16 dx, i16 dy);
void set_view_mode(void);
void set_zoom_level(i16 level);
i16 create_me(void);
void continent_check(void);

#endif
