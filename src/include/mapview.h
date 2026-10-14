/* mapview.h -- the editor's screen: redrawing it whole, the blinking cursor, the menu bar.
 *
 * Defined in ui/mapview.c. main_screen_refresh clears the work screen and redraws the menu
 * bar, the map and the info window; cursor_update toggles the cursor square on the map.
 * The map view's geometry and the terrain drawing are declared in map.h.
 */
#ifndef MAPEDIT_MAPVIEW_H
#define MAPEDIT_MAPVIEW_H

#include "msc.h"
#include "menu.h"
void main_screen_refresh(void);
void cursor_update(void);
extern Menu *menu_bar;         /* the editor's menu bar (construct_mapedit_menu) */

#endif
