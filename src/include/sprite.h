/* sprite.h -- sprite series: MicroProse's .SS files loaded (sprite_e.c) and drawn
 * (port/lib/sprite.c), and the run-length frames inside them.
 *
 * Callers number frames from 1: frame n is icon[n - 1]; bit 15 set (n | 8000h) draws it
 * mirrored. A frame's pixels are rows opening with a mode byte (port/lib/sprite.c has the
 * format).
 */
#ifndef MAPEDIT_SPRITE_H
#define MAPEDIT_SPRITE_H

#include "msc.h"

#pragma pack(push, 2)
typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;      /* one frame */
typedef struct { i32 offset; i32 size; } FrameSize;                       /* a packed frame's place */

typedef struct {                /* a packed series: where its frames are read from */
    char kind;                  /* the loader's kind: 0, on disk */
    char b1;                    /* header byte 1 */
    MSC_FILE *fp;
    i32 pos;                    /* file position of the frame data */
} SpriteInfo;

typedef struct {
    i16 flag0;
    i16 x2;
    i16 count;                  /* frames */
    i16 pal_handle;
    i16 tab[16];                /* the header's words: tab[2] marks the extra block, tab[3] no
                                   palette; a packed series keeps its largest frame in tab[0..1];
                                   tab[4..6] are an animation's offsets (popup.c) */
    i16 x28;
    i16 x2a;
    char packed;
    char pad2d;
    SpriteInfo *info;           /* a packed series' */
    FrameSize *sizes;           /* a packed series' */
    char *color_flags;
    char *arena;                /* a packed series' unpacking buffer */
    char *extra;                /* the 0x68-byte extra block, when tab[2] */
    IconRec icon[1];            /* count of them */
} SpriteSeries;
#pragma pack(pop)

SpriteSeries *sprite_series_load(char *name, i16 flags);
void sprite_color_translate(SpriteSeries *sheet, char *map);
void sprite_draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y);
void sprite_draw_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y);
void sprite_draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale);
void sprite_draw_scaled_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale);

extern i16 sprite_error;
extern char *put_palette;
extern char *sprite_force_memory;
extern i32 sprite_last_quantity;
extern i32 sprite_force_size;

#endif
