/* video.h -- the functions, data and records of video.c. */
#ifndef MAPEDIT_VIDEO_H
#define MAPEDIT_VIDEO_H

#include "msc.h"

char *VIDEO_ADDRESS(i16 x, i16 y);
void video_init(i16 mode, i16 set);
void video_update(DosPort *port, i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);
extern i16 video_mode;
void MCGA_SHAKE(void);
extern i16 MCGA_SHAKES;

#endif
