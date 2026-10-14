/* screen.h -- the screens the editor draws into.
 *
 * Defined in sys/screen.c. Each screen is a DosPort (buffer.h). scr_video is the 320 x 200
 * mode 13h screen itself; the editor allocates two more at startup, 320 x 200 each: the map
 * view is rendered into scr_orig, and scr_work is the map with the cursor, the menus and the
 * popups drawn over it, which goes to the screen. The others belong to the game's shared
 * code and stay empty here.
 */
#ifndef MAPEDIT_SCREEN_H
#define MAPEDIT_SCREEN_H

#include "msc.h"

void screen_dominant_mode(i16 mode);

extern DosPort scr_depth;       /* unused */
extern DosPort scr_main;        /* unused */
extern DosPort scr_orig;        /* the map view as rendered, without overlays */
extern DosPort scr_special;     /* unused */
extern DosPort scr_video;       /* the screen */
extern DosPort scr_walk;        /* unused */
extern DosPort scr_work;        /* what goes to the screen: the map plus overlays */

#endif
