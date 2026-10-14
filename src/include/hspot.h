/* hspot.h -- hot spots: the screen rectangles a click is matched against.
 *
 * Defined in ui/hotspot.c. spot[] holds up to 120 rectangles, numbered from 1 (entry 0 is
 * unused), each with two values for its owner, an id that groups one screen's spots and an
 * active flag (HotSpot, port/port_records.h). numspots is the last entry in use; a screen
 * empties the table by setting it to 0. hspot_add appends a spot and hspot_which finds the
 * one with a given id under a point. Map Tile Select is the only user.
 */
#ifndef MAPEDIT_HSPOT_H
#define MAPEDIT_HSPOT_H

#include "msc.h"

i16 hspot_add(i16 x0, i16 y0, i16 x1, i16 y1, i16 v1, i16 v2, i16 id);

extern i16 numspots;
extern HotSpot spot[121];
i16 hspot_which(u16 x, u16 y, i16 id);

#endif
