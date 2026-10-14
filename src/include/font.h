/* font.h -- fonts: loading .FF files, their colours, writing and measuring text.
 *
 * Defined in ui/font.c. A font is a whole .FF file in memory: a height byte, a width byte
 * per character and the offset of each glyph, whose pixels are two bits each and pick one of
 * the four font_colors (FFh is transparent). The editor loads two fonts at startup:
 * font_inter (FONTINTR) for popups and text, and menu_font (FONTTINY, theme.h) for menus and
 * panels. The other font pointers stay null.
 */
#ifndef MAPEDIT_FONT_H
#define MAPEDIT_FONT_H

#include "msc.h"

#pragma pack(push, 2)
/* A loaded font, seen through its first byte: the height of every glyph in pixels. */
typedef struct { u8 height; } DosFont;
#pragma pack(pop)

i16 font_write(DosPort *port, i16 x, i16 y, DosFont *font_, char *s, i16 gap);
void *font_load(char *name);
void font_set_colors(i16 a, i16 b, i16 c, i16 d);

extern void *font_conv;         /* unused */
extern void *font_inter;        /* the interface font (FONTINTR) */
extern void *font_main;         /* unused */
extern void *font_menu;         /* unused */
extern void *font_misc;         /* unused */
extern u8 font_colors[4];       /* what pixel values 0..3 draw as; FFh is transparent */
i16 font_string_width(i16 spacing, DosFont *font_, char *s);

#endif
