/* editstate.h -- the editing state: the cursor, the selection, the brush, undo.
 *
 * Defined in core/editstate.c: the state the views show and the commands change. Map Tile
 * Select (ui/tilepicker.c) sets the selection, the paint commands (editor/edit.c) apply it,
 * and the info panel shows it. make_undo saves the terrain plane before an edit and
 * perform_undo puts it back: one level of undo.
 */
#ifndef MAPEDIT_EDITSTATE_H
#define MAPEDIT_EDITSTATE_H

#include "msc.h"
extern i16 map_changes;                 /* the map has unsaved changes */
extern i16 small_map_needs_update;      /* unused */
extern i16 fill_radius;                 /* the brush: 0, 1 or 2 squares around the centre */
extern i16 map_selected;                /* the command line named the map (-m:file) */
extern i16 coastline_protect;           /* painting leaves ocean and sea lane alone */
extern i16 undo_available;              /* the undo buffer was allocated */
extern i16 undo_active;                 /* the undo buffer holds a saved map */
extern i16 create_me_now;               /* create a new map instead of loading one */
extern char selected_terrain;           /* the terrain painted */
extern u8 selected_physical_and_mask;   /* the terrain byte's high bits painting keeps */
extern u8 selected_physical_or_mask;    /* the high bits it sets: hills, mountains, river */
extern char selected_right_mouse_change;    /* an overlay is selected: keep the terrain */
extern void *map_undo_memory;           /* the undo buffer: a copy of the terrain plane */
extern i16 map_undo_ems_handle;         /* unused */
extern i16 cursor_x;                    /* the cursor's square */
extern i16 cursor_y;
extern i16 cursor_on;                   /* the cursor square is drawn at the moment */
void make_undo(void);
void perform_undo(void);

#endif
