/* distance.c -- distances and directions between squares. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "loader.h"
#include "mapedit.h"
#include "records.h"
#include "units.h"
/* MODULE stuff: MAPEDIT.EXE segment 0865 (stuff.obj), file 0x9c5e-0x9db4, 7 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn minimax          0x09c5e
 * fn swap             0x09c7c
 * fn xy_dist          0x09c92
 * fn map_dist         0x09cce
 * fn xy_dist_2        0x09d16
 * fn map_dist_2       0x09d46
 * fn bearing_adjacent 0x09d8e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_024c).
 *
 * SMALL ARITHMETIC (root code, frame 024c). Windows twins: `minimax`, swap.c,
 * xy_dist.c, map_dist.c. DOS adds the max-of-absolutes (Chebyshev) pair xy_dist_2 /
 * map_dist_2 and bearing_adjacent(a, b): whether direction a is next to b (mod 8).
 *
 * The whole file is "elg" (/Ox-style engine code): registers by lifetime throughout.
 */

/* minimax -- 0865:000e, 30 bytes */
i16 minimax(i16 value, i16 lo, i16 hi)
{
    value = max(value, lo);
    value = min(value, hi);
    return value;
}
/* swap -- 0865:002c, 22 bytes */
void swap(i16 *a, i16 *b)
{
    i16 t;

    t = *a;
    *a = *b;
    *b = t;
}
/* xy_dist -- 0865:0042, 60 bytes */
i16 xy_dist(i16 dx, i16 dy)
{
    i16 r;

    dx = dx > 0 ? dx : ~dx + 1;
    dy = dy > 0 ? dy : ~dy + 1;
    if (dx > dy)
        r = (dy >> 1) + dx;
    else
        r = (dx >> 1) + dy;
    return r;
}
/* map_dist -- 0865:007e, 72 bytes */
i16 map_dist(i16 x1, i16 y1, i16 x2, i16 y2)
{
    i16 dx;
    i16 dy;

    dx = x1 - x2 > 0 ? x1 - x2 : ~(x1 - x2) + 1;
    dy = y1 - y2 > 0 ? y1 - y2 : ~(y1 - y2) + 1;
    return xy_dist(dx, dy);
}
/* xy_dist_2 -- 0865:00c6, 48 bytes */
i16 xy_dist_2(i16 dx, i16 dy)
{
    i16 r;

    dx = dx > 0 ? dx : ~dx + 1;
    dy = dy > 0 ? dy : ~dy + 1;
    if (dx > dy)
        r = dx;
    else
        r = dy;
    return r;
}
/* map_dist_2 -- 0865:00f6, 72 bytes */
i16 map_dist_2(i16 x1, i16 y1, i16 x2, i16 y2)
{
    i16 dx;
    i16 dy;

    dx = x1 - x2 > 0 ? x1 - x2 : ~(x1 - x2) + 1;
    dy = y1 - y2 > 0 ? y1 - y2 : ~(y1 - y2) + 1;
    return xy_dist_2(dx, dy);
}
/* bearing_adjacent -- 0865:013e, 38 bytes */
i16 bearing_adjacent(i16 a, i16 b)
{
    i16 r;

    r = 0;
    if (((b + 1) & 7) == a || ((b - 1) & 7) == a)
        r = 1;
    return r;
}

