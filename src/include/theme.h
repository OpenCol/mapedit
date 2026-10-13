/* theme.h */
#ifndef MAPEDIT_THEME_H
#define MAPEDIT_THEME_H

#include "msc.h"
#include "font.h"
extern u8 basic_color;
extern u8 hilite_color;
extern u8 grey_color;
extern u8 enhance_color;
extern u8 shadow_color;
extern u8 select_color;
extern u8 notice_color;
extern u8 border0;
extern u8 border1;
extern u8 border2;
extern DosFont *menu_font;
void popups_normal(void);
void theme_load(void);

#endif
