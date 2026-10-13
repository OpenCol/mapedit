/* tilepicker.h */
#ifndef MAPEDIT_TILEPICKER_H
#define MAPEDIT_TILEPICKER_H

#include "msc.h"
extern i16 myspot;
void add_item(i16 x, i16 y, i16 icon, i16 n, i16 sel);
void add_item_2(i16 x, i16 y, i16 tile, i16 n, i16 sel);
void selection_screen_display(void);
void parse_spot(i16 n);
i16 selection_mouse(void);
void selection_screen(void);

#endif
