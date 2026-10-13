/* hspot.h -- the functions, data and records of hspot_1.c, misc.c. */
#ifndef MAPEDIT_HSPOT_H
#define MAPEDIT_HSPOT_H

#include "msc.h"

i16 hspot_which(u16 x, u16 y, i16 id);
i16 hspot_add(i16 x0, i16 y0, i16 x1, i16 y1, i16 v1, i16 v2, i16 id);

extern i16 numspots;
extern HotSpot spot[121];

#endif
