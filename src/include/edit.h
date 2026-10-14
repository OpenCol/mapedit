/* edit.h -- the editing commands: painting, the brush, the cursor, the view's centre and zoom.
 *
 * Defined in editor/edit.c. change_map paints one square with the current selection (or picks
 * the square's terrain up as the selection) and fill_map does it for the whole brush and
 * redraws; set_center, possibly_center, move_cursor and set_zoom_level move the view and the
 * cursor; create_me makes a new blank map and continent_check runs Find Continents.
 */
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
