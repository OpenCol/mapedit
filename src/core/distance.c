/* distance.c -- distances and directions between squares, and two small helpers.
 *
 * Two distances: an approximation of the straight-line one (xy_dist, map_dist), the longer
 * axis plus half the shorter, and the larger of the two axes (xy_dist_2, map_dist_2).
 * bearing_adjacent counts directions 0-7, wrapping round. The module is shared with the game;
 * the editor calls only minimax.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "loader.h"
#include "mapedit.h"
#include "records.h"
#include "units.h"

/* value clamped to lo..hi; hi wins if lo > hi. */
i16 minimax(i16 value, i16 lo, i16 hi)
{
    value = max(value, lo);
    value = min(value, hi);
    return value;
}

void swap(i16 *a, i16 *b)
{
    i16 t;

    t = *a;
    *a = *b;
    *b = t;
}

/* The distance of an offset (dx, dy): the longer axis plus half the shorter, rounded down. */
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

/* xy_dist between two squares. */
i16 map_dist(i16 x1, i16 y1, i16 x2, i16 y2)
{
    i16 dx;
    i16 dy;

    dx = x1 - x2 > 0 ? x1 - x2 : ~(x1 - x2) + 1;
    dy = y1 - y2 > 0 ? y1 - y2 : ~(y1 - y2) + 1;
    return xy_dist(dx, dy);
}

/* The distance of an offset (dx, dy) in king's moves: the longer axis. */
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

/* xy_dist_2 between two squares. */
i16 map_dist_2(i16 x1, i16 y1, i16 x2, i16 y2)
{
    i16 dx;
    i16 dy;

    dx = x1 - x2 > 0 ? x1 - x2 : ~(x1 - x2) + 1;
    dy = y1 - y2 > 0 ? y1 - y2 : ~(y1 - y2) + 1;
    return xy_dist_2(dx, dy);
}

/* Is direction a one step (45 degrees) either side of direction b? 0 when they are equal. */
i16 bearing_adjacent(i16 a, i16 b)
{
    i16 r;

    r = 0;
    if (((b + 1) & 7) == a || ((b - 1) & 7) == a)
        r = 1;
    return r;
}

