/* font.h -- the functions, data and records of font.c, font_1.c, font_2.c, font_3.c, rest.c. */
#ifndef MAPEDIT_FONT_H
#define MAPEDIT_FONT_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct { u8 height; } DosFont;
#pragma pack(pop)

i16 font_write(DosPort *port, i16 x, i16 y, DosFont *font_, char *s, i16 gap);
void *font_load(char *name);
void font_set_colors(i16 a, i16 b, i16 c, i16 d);

extern void *font_conv;
extern void *font_inter;
extern void *font_main;
extern void *font_menu;
extern void *font_misc;
extern u8 font_colors[4];
i16 font_string_width(i16 spacing, DosFont *font_, char *s);

#endif
