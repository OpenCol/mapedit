/* theme.h -- the editor's colours and its small font.
 *
 * Defined in ui/theme.c. The colours are palette indices. They start at defaults; theme_load
 * replaces nine of them from the COLORS section of NAMES.TXT and hands them to the popups
 * (popups_normal). The editor's main copies the same values into the menu colours (menu.h).
 */
#ifndef MAPEDIT_THEME_H
#define MAPEDIT_THEME_H

#include "msc.h"
#include "font.h"
extern u8 basic_color;          /* ordinary text */
extern u8 hilite_color;         /* hotkeys and {highlighted} spans */
extern u8 grey_color;           /* greyed (disabled) items */
extern u8 enhance_color;        /* the font's third colour */
extern u8 shadow_color;         /* the font's fourth colour */
extern u8 select_color;         /* the bar behind the selected item */
extern u8 notice_color;         /* not read */
extern u8 border0;              /* the popup frame's outer box */
extern u8 border1;              /* its inner top and right lines */
extern u8 border2;              /* its inner left and bottom lines */
extern DosFont *menu_font;      /* the small font (FONTTINY) of menus and panels */
void popups_normal(void);
void theme_load(void);

#endif
