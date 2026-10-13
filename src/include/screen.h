/* screen.h -- the functions, data and records of matte_0.c, screen_b.c. */
#ifndef MAPEDIT_SCREEN_H
#define MAPEDIT_SCREEN_H

#include "msc.h"

void screen_dominant_mode(i16 mode);

extern DosPort scr_depth;
extern DosPort scr_main;
extern DosPort scr_orig;
extern DosPort scr_special;
extern DosPort scr_video;
extern DosPort scr_walk;
extern DosPort scr_work;

#endif
