/* video.h -- the mode 13h screen: 320 x 200 palette indices, and the screen shake.
 *
 * Defined in sys/video.c. The screen is the host's frame buffer, which the host shows at its
 * next 70 Hz frame after a write. VIDEO_ADDRESS is a pixel's place in it; video_update
 * copies a rectangle of a work buffer to it.
 */
#ifndef MAPEDIT_VIDEO_H
#define MAPEDIT_VIDEO_H

#include "msc.h"

char *VIDEO_ADDRESS(i16 x, i16 y);
void video_init(i16 mode, i16 set);
void video_update(DosPort *port, i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);
extern i16 video_mode;          /* the mode last passed to video_init */

#endif
