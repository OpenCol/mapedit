/* hotspot.c -- hot spots: the screen rectangles a click is matched against. */
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
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE hspot_1: MAPEDIT.EXE segment 0ba3 (hspot_1.c), file 0xd030-0xd0a2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn hspot_add    0x0d030
 *
 * No VICEROY twin: reconstructed from the shipped bytes. "elg"; numspots is used directly
 * (cached in si, written back at the end): a local copy splits its range into cx.
 *
 * HOT SPOTS. The table `spot` (DS:0x530a, 16-byte records) holds up to 120 rectangles
 * with two user values and an id; `numspots` (DS:0x0716) is the index of the last one
 * (entry 0 is unused). hspot_add(x0, y0, x1, y1, v1, v2, id) (_fastcall) appends one,
 * active, and returns -1; when the table is full it reports error -0x44 (level 2, module
 * 0x1c, with v1 and v2 as the data words) and returns 0.
 */

i16 numspots = 0;                                /* DS:0x0716 (defined here) */
HotSpot spot[121];                               /* DS:0x530a (BSS, 1936 bytes) */
/* hspot_add -- 0ba3:0000, 114 bytes */
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

/* hspot_which -- 0baa:0002: the first active hot spot (from 1) with this id whose rectangle
   holds (x, y), compared unsigned; 0 when none does */
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

/* MODULE hspot_7: MAPEDIT.EXE segment 0baa (HSPOT_7.C), file 0xd0a2-0xd0f4, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn hspot_which  0x0d0a2
 *
 * No VICEROY twin: reconstructed from the shipped bytes. The body is inline assembly (the
 * prologue homes all three register parameters); the nop before `none:` is MSC's own
 * padding of a label that follows an unconditional jmp to an even offset.
 *
 * hspot_which(x, y, id) (_fastcall) returns the index (from 1) of the first active hot
 * spot with that id containing (x, y), or 0. The coordinates compare unsigned.
 */



/* hspot_which -- 0baa:0002, 82 bytes */
/* PORT: hspot_which is in port/ (inline assembly) */
