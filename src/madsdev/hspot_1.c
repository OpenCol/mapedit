/* Imported from mapedit-decomp src/madsdev/hspot_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
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

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 x0; i16 y0; i16 x1; i16 y1; i16 v1; i16 v2; i16 id; i16 active; } HotSpot;

extern void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2);
extern i16 numspots;                             /* DS:0x0716 */
extern HotSpot spot[];                           /* DS:0x530a */
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

