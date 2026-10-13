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

typedef struct { u8 *data; i16 x0; i16 y0; i16 w; i16 h; } IconRec;

/* only the frame table matters here; the series' layout up to it is sprite_e.c's */
typedef struct {
    i16 flag0, x2, count, pal_handle, tab[16], x28, x2a;
    char packed, pad2d;
    void *info, *sizes;
    char *color_flags, *arena, *extra;
    IconRec icon[1];
} SpriteSeriesView;

static IconRec *frame(void *sheet, i16 n)
{
    return &((SpriteSeriesView *)sheet)->icon[n - 1];
}

/* the shared decoder: kind 0 draws colours, 1 draws black (sprite_draw_black) */
static void draw(i16 icon, void *sheet, DosPort *port, i16 x, i16 y, int black)
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
                                if (c != 0xfd)
                                    *d = black ? 0 : (char)c;
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
                                    if (c != 0xfd)
                                        *d = black ? 0 : (char)c;
                                    d += dir;
                                }
                                col++;
                            } while (--n);
                            continue;
                        }
                        if (col >= first && col < last) {
                            if (c != 0xfd)
                                *d = black ? 0 : (char)c;
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
void sprite_draw(i16 icon, void *sheet, DosPort *port, i16 x, i16 y)
{
    draw(icon, sheet, port, x, y, 0);
}
