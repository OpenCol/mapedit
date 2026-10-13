/* sprite.c -- the sprite drawers of sprite_1.c (and their kin), MicroProse's run-length
 * sprites, whose bodies were one block of inline assembly each.
 *
 * A sprite series (sprite_e.c's SpriteSeries) has a record per frame: its pixels (a pointer),
 * and its width and height. Callers number frames from 1: the original reaches frame n's
 * record at sheet + 36h + 12n, the series' icon[n - 1]. Bit 15 set (n | 8000h) draws the
 * frame mirrored left to right.
 *
 * A frame's pixels are rows of bytes, each row opening with a mode byte:
 *   FFh  an empty row (nothing follows);
 *   FDh  runs: (count, colour) pairs, until FFh ends the row;
 *   else plain: one colour a byte, FEh count colour a run, FFh ends the row.
 * Colour FDh is transparent. Only the columns inside the clip are drawn, and only they move
 * the destination; a row cut short by the clip skips its remaining bytes up to its FFh. All
 * of that, the clip arithmetic of a mirrored frame included, is the original's, step for
 * step.
 */
#include "msc.h"
#include "records.h"
#include "units.h"

#include "sprite.h"

static IconRec *frame(SpriteSeries *sheet, i16 n)
{
    return &sheet->icon[n - 1];
}

/* the shared decoder; with under set (sprite_draw_black) a pixel is written only where the
   destination is still 0, under what is drawn already -- the destination moves either way */
static void draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, int under)
{
    i16 dir = 1, maxx1 = (i16)(port->w - 1), maxy = (i16)(port->h - 1);
    i16 iwo, ihn, skipx, visw, endx, skipyo, vish, endyo, bx, cx, ax, dx;
    IconRec *rec;
    const u8 *s;
    char *row;
    u16 r;

    if (icon < 0)
        dir = -1;
    icon = (i16)(icon & 0x7fff);
    rec = frame(sheet, icon);
    s = rec->data;
    iwo = rec->w;
    ihn = rec->h;

    /* columns */
    bx = iwo;
    cx = 0;
    ax = x;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxx1);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipx = cx;
    visw = bx;
    cx = (i16)(cx + bx);
    endx = cx;
    if (bx <= 0)
        return;
    if (dir != 1) {
        i16 di = iwo;
        ax = (i16)(ax + di - 1);
        x = ax;
        di = (i16)(di - cx);
        di = (i16)-di;
        skipx = di;
        di = (i16)-di;
        di = (i16)(di + visw);
        endx = di;
    }

    /* rows */
    bx = ihn;
    cx = 0;
    ax = y;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxy);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipyo = cx;
    vish = bx;
    endyo = (i16)(cx + bx);
    if (bx <= 0)
        return;

    row = port->bits + (u32)(u16)(cx + ax) * (u16)port->w;
    row += x + skipx;
    {
        i16 first = (i16)(skipx * dir), last = endx;
        i16 rowno;
        for (rowno = 0; rowno < endyo; rowno++) {
            int eol = 0;
            if (rowno >= skipyo) {
                char *d = row;
                i16 col = 0;
                u8 mode = *s++;
                if (mode == 0xff)
                    eol = 1;
                else if (mode == 0xfd) {
                    while (col < last) {
                        u8 n = *s++, c;
                        if (n == 0xff) {
                            eol = 1;
                            break;
                        }
                        c = *s++;
                        do {
                            if (col >= first && col < last) {
                                if (c != 0xfd && !(under && *d))
                                    *d = (char)c;
                                d += dir;
                            }
                            col++;
                        } while (--n);
                    }
                } else {
                    while (col < last) {
                        u8 c = *s++;
                        if (c == 0xff) {
                            eol = 1;
                            break;
                        }
                        if (c == 0xfe) {
                            u8 n = *s++;
                            c = *s++;
                            do {
                                if (col >= first && col < last) {
                                    if (c != 0xfd && !(under && *d))
                                        *d = (char)c;
                                    d += dir;
                                }
                                col++;
                            } while (--n);
                            continue;
                        }
                        if (col >= first && col < last) {
                            if (c != 0xfd && !(under && *d))
                                *d = (char)c;
                            d += dir;
                        }
                        col++;
                    }
                }
                row += port->w;
            }
            if (!eol)
                while (*s++ != 0xff)
                    ;
        }
    }
    (void)vish;
    (void)r;
}

/* sprite_draw -- 0d8f:0000 */
void sprite_draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y)
{
    draw(icon, sheet, port, x, y, 0);
}

/* sprite_draw_black -- 0e71:000c: the same, only onto pixels that are 0 */
void sprite_draw_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y)
{
    draw(icon, sheet, port, x, y, 1);
}

/* sprite_draw_scaled -- 0dae:000a: the frame shrunk to scale percent. A table over the
   frame's larger dimension keeps a source row or column when an accumulator (from 50, plus
   scale each step) reaches 100; the output is centred on x and its bottom row is y. Dropped
   columns move only the source; kept ones are clipped and drawn as sprite_draw's are. */
static void draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale, int under)
{
    i16 dir = 1, maxx1 = (i16)(port->w - 1), maxy = (i16)(port->h - 1);
    i16 iwo, ihn, maxdim, outw = 0, outh = 0, skipx, visw, endx, skipyo, endyo;
    i16 bx, cx, ax, dx, di, acc;
    u8 keep[320];
    IconRec *rec;
    const u8 *s;
    char *row;

    if (icon < 0)
        dir = -1;
    icon = (i16)(icon & 0x7fff);
    rec = frame(sheet, icon);
    s = rec->data;
    iwo = rec->w;
    ihn = rec->h;
    maxdim = iwo >= ihn ? iwo : ihn;
    acc = 50;
    for (di = 0; di < maxdim; di++) {
        acc = (i16)(acc + scale);
        if (acc >= 100) {
            if (di < (i16)sizeof keep)
                keep[di] = 0xff;
            acc = (i16)(acc - 100);
            if (di < iwo)
                outw++;
            if (di < ihn)
                outh++;
        } else if (di < (i16)sizeof keep)
            keep[di] = 0;
    }
    x = (i16)(x - ((u16)outw >> 1));
    y = (i16)(y - outh + 1);

    bx = outw;
    cx = 0;
    ax = x;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxx1);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipx = cx;
    visw = bx;
    cx = (i16)(cx + bx);
    endx = cx;
    if (bx <= 0)
        return;
    if (dir != 1) {
        i16 d2 = outw;
        ax = (i16)(ax + d2 - 1);
        x = ax;
        d2 = (i16)(d2 - cx);
        d2 = (i16)-d2;
        skipx = d2;
        d2 = (i16)-d2;
        d2 = (i16)(d2 + visw);
        endx = d2;
    }
    bx = outh;
    cx = 0;
    ax = y;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxy);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipyo = cx;
    endyo = (i16)(cx + bx);
    if (bx <= 0)
        return;

    row = port->bits + (u32)(u16)(cx + ax) * (u16)port->w;
    row += x + skipx;
    skipx = (i16)(skipx * dir);
    {
        i16 srow, orow = -1;
        for (srow = 0; srow < ihn; srow++) {
            int eol = 0;
            if (keep[srow]) {
                if (++orow >= endyo)
                    return;
                if (orow >= skipyo) {
                    char *d = row;
                    i16 col = 0, out = 0;
                    u8 mode = *s++;
#define PIXEL(c) do { \
        if (keep[col]) { \
            if (out >= skipx && out < endx) { \
                if ((c) != 0xfd && !(under && *d)) \
                    *d = (char)(c); \
                d += dir; \
            } \
            out++; \
        } \
        col++; \
    } while (0)
                    if (mode == 0xff)
                        eol = 1;
                    else if (mode == 0xfd) {
                        while (out < endx) {
                            u8 n = *s++, c;
                            if (n == 0xff) {
                                eol = 1;
                                break;
                            }
                            c = *s++;
                            do PIXEL(c); while (--n);
                        }
                    } else {
                        while (out < endx) {
                            u8 c = *s++;
                            if (c == 0xff) {
                                eol = 1;
                                break;
                            }
                            if (c == 0xfe) {
                                u8 n = *s++;
                                c = *s++;
                                do PIXEL(c); while (--n);
                                continue;
                            }
                            PIXEL(c);
                        }
                    }
#undef PIXEL
                    row += port->w;
                }
            }
            if (!eol)
                while (*s++ != 0xff)
                    ;
        }
    }
}

void sprite_draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale)
{
    draw_scaled(icon, sheet, port, x, y, scale, 0);
}

/* sprite_draw_scaled_black -- 0e92:000a: scaled, only onto pixels that are 0 */
void sprite_draw_scaled_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale)
{
    draw_scaled(icon, sheet, port, x, y, scale, 1);
}
