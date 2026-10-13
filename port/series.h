/* series.h -- the sprite series record (sprite_e.c's SpriteSeries) as the port's drawers see
 * it. Only the frame table matters to them. The imported modules lay their records out on
 * 2-byte boundaries (MSC 6's /Zp2, a #pragma pack(2) in each), so this view must too: with
 * the compiler's own alignment its pointers, and the frame table after them, sit 2 bytes
 * later than where sprite_series_load put them.
 */
#ifndef MAPEDIT_SERIES_H
#define MAPEDIT_SERIES_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;

typedef struct {
    i16 flag0, x2, count, pal_handle, tab[16], x28, x2a;
    char packed, pad2d;
    void *info, *sizes;
    char *color_flags, *arena, *extra;
    IconRec icon[1];
} SpriteSeriesView;
#pragma pack(pop)

#endif
