/* sprite.h -- sprite series: MicroProse's .SS files, loaded and drawn.
 *
 * Defined in ui/sprite.c. A sprite series is a set of frames sharing a colour list, loaded
 * from a MADSPACK file (loader.h) into one block: the SpriteSeries record with an IconRec per
 * frame, then the pixels. On loading, the colours are given places in the palette (pal.h)
 * and every pixel is rewritten to its colour's palette index (sprite_color_translate).
 *
 * Callers number frames from 1: frame n is icon[n - 1]; bit 15 set (n | 8000h) draws it
 * mirrored. A frame's pixels are rows that each open with a mode byte (sprite.c has the
 * format); colour FDh is transparent. A packed series keeps its frames in the file and
 * unpacks each one when it is drawn.
 */
#ifndef MAPEDIT_SPRITE_H
#define MAPEDIT_SPRITE_H

#include "msc.h"

#pragma pack(push, 2)
/* A frame: its pixels (null in a packed series), x0 and y0 from the file's frame table (not
 * used in drawing), its width and height. */
typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;
typedef struct { i32 offset; i32 size; } FrameSize;     /* a packed frame's place in the file */

typedef struct {                /* a packed series: where its frames are read from */
    char kind;                  /* where the file is: always 0, on disk */
    char b1;                    /* the header's second byte */
    MSC_FILE *fp;
    i32 pos;                    /* file position of the frame data */
} SpriteInfo;

typedef struct {
    i16 flag0;                  /* the header's words 1 and 2 say so (word 1 set, word 2 < 4);
                                   not read */
    i16 x2;                     /* the header's word 2; not read */
    i16 count;                  /* frames */
    i16 pal_handle;             /* the palette flag pal_allocate gave its colours, or 0 */
    i16 tab[16];                /* the header's words: tab[2] marks the extra block, tab[3] no
                                   palette; a packed series keeps its largest frame's size in
                                   tab[0..1]; tab[4..6] place a portrait by a popup (popup.c) */
    i16 x28;                    /* two more header words; not read */
    i16 x2a;
    char packed;                /* the frames stay in the file */
    char pad2d;
    SpriteInfo *info;           /* a packed series' */
    FrameSize *sizes;           /* a packed series' */
    char *color_flags;          /* a packed series': each colour's palette index */
    char *arena;                /* a packed series' unpacking buffer */
    char *extra;                /* the 0x68-byte extra block, when tab[2] */
    IconRec icon[1];            /* count of them */
} SpriteSeries;
#pragma pack(pop)

SpriteSeries *sprite_series_load(char *name, i16 flags);
void sprite_draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y);
void sprite_draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale);

extern i16 sprite_error;        /* why the last sprite_series_load failed */
extern char *put_palette;       /* where a series without a colour list puts its palette */
extern char *sprite_force_memory;   /* a block to load the next series into, or 0 */
extern i32 sprite_last_quantity;    /* the bytes the last series needed */
extern i32 sprite_force_size;       /* the size of sprite_force_memory */
void sprite_color_translate(SpriteSeries *sheet, char *map);

#endif
