/* hotspot.c -- hot spots: the screen rectangles a click is matched against.
 *
 * The table spot[] holds up to 120 rectangles, each with two values for its owner (v1, v2),
 * an id that groups the spots of one screen, and an active flag. Entries are numbered from 1;
 * numspots is the number of the last one in use, and a screen clears the table by setting
 * numspots to 0. hspot_add appends a spot; hspot_which finds the one under a point.
 */
#include "msc.h"
#include "error.h"
#include "font.h"
#include "hspot.h"
#include "loader.h"
#include "menu.h"
#include "pack.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as in 1994; file formats rely on it */

i16 numspots = 0;                                /* the last entry in use; 0 when empty */
HotSpot spot[121];                               /* entry 0 is unused */

/* hspot_add -- append an active hot spot covering (x0, y0)-(x1, y1), corners included, with
 * the values v1, v2 and the id. Returns -1, or 0 when the table already holds 120 spots, after
 * reporting the error (-0x44, with v1 and v2 as its data).
 */
i16 hspot_add(i16 x0, i16 y0, i16 x1, i16 y1, i16 v1, i16 v2, i16 id)
{
    if (numspots == 120) {
        error_report(-0x44, 2, 0x1c, (i32)v1, (i32)v2);
        return 0;
    }
    numspots++;
    spot[numspots].x0 = x0;
    spot[numspots].y0 = y0;
    spot[numspots].x1 = x1;
    spot[numspots].y1 = y1;
    spot[numspots].v1 = v1;
    spot[numspots].v2 = v2;
    spot[numspots].active = -1;
    spot[numspots].id = id;
    return -1;
}

/* hspot_which -- the number of the first active hot spot with this id whose rectangle holds
 * (x, y), corners included; 0 when none does. The comparisons are unsigned: a negative
 * coordinate counts as a very large one.
 */
i16 hspot_which(u16 x, u16 y, i16 id)
{
    u16 i;
    for (i = 1; i <= (u16)numspots; i++) {
        HotSpot *s = &spot[i];
        if (s->active == 0)
            continue;
        if (x < (u16)s->x0 || x > (u16)s->x1 || y < (u16)s->y0 || y > (u16)s->y1)
            continue;
        if (id == s->id)
            return (i16)i;
    }
    return 0;
}
