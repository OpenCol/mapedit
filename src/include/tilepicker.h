/* tilepicker.h -- Map Tile Select: the screen of terrains and features to paint with.
 *
 * Defined in ui/tilepicker.c. selection_screen replaces the work screen with rows of 16 x 16
 * pictures, each a hot spot: the terrains, then the features (rivers, mountains, hills).
 * Picking one sets the selection in the editing state (editstate.h) that the paint commands
 * use (parse_spot); the mouse or the arrow keys pick, and leaving returns to the map.
 */
#ifndef MAPEDIT_TILEPICKER_H
#define MAPEDIT_TILEPICKER_H

#include "msc.h"
extern i16 myspot;              /* the hot spot the arrow keys move from */
void add_item(i16 x, i16 y, i16 icon, i16 n, i16 sel);
void add_item_2(i16 x, i16 y, i16 tile, i16 n, i16 sel);
void selection_screen_display(void);
void parse_spot(i16 n);
i16 selection_mouse(void);
void selection_screen(void);

#endif
