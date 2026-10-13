/* edit.h */
#ifndef MAPEDIT_EDIT_H
#define MAPEDIT_EDIT_H

#include "msc.h"
extern i16 map_changes;
extern i16 small_map_needs_update;
extern i16 fill_radius;
extern i16 map_selected;
extern i16 coastline_protect;
extern i16 undo_available;
extern i16 undo_active;
extern i16 create_me_now;
extern char selected_terrain;
extern u8 selected_physical_and_mask;
extern u8 selected_physical_or_mask;
extern char selected_right_mouse_change;
extern void *map_undo_memory;
extern i16 map_undo_ems_handle;
extern i16 cursor_x;
extern i16 cursor_y;
void make_undo(void);
void perform_undo(void);
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
