/* font.c -- font_2.c's font_write, whose glyph loop was inline assembly.
 *
 * A font: byte 0 the height; from +2 a width a character (index = code - 1); from +82h a
 * word a character, the offset of its glyph from the font's start. A glyph is height rows of
 * width pixels, two bits each, four to a byte, high bits first, each row starting on a byte;
 * a pixel's colour is font_colors[bits], FFh leaving the destination as it is.
 *
 * The string ends at a 0, and also at any code from 81h up (the original decrements the code
 * and stops on the sign). A character that would pass the buffer's width stops the string;
 * an empty-width character draws nothing and adds no gap. Rows above the buffer are skipped
 * in the glyph, rows below it clipped. Returns the x after the last character drawn.
 */
#include <string.h>

#include "msc.h"

typedef struct { u8 c[4]; } Colours;
extern Colours font_colors;
char *buffer_pointer(DosPort *port, i16 x, i16 y);

i16 font_write(DosPort *port, i16 x, i16 y, u8 *font, char *s, i16 gap)
{
    char buf[80];
    Colours cols = font_colors;
    i8 rows;
    i16 bottom, skip = 0, clip, cursor;
    char *dst, *d;
    const u8 *p;
    u16 n = 0;

    while (n < sizeof buf - 1 && s[n]) {        /* PORT: the original copies without a limit */
        buf[n] = s[n];
        n++;
    }
    buf[n] = 0;
    if (y < 0) {
        skip = (i16)-y;
        y = 0;
    }
    rows = (i8)((i8)font[0] - skip >= 0 ? (i8)font[0] - skip : 0);
    bottom = (i16)(rows + y - 1);
    if (bottom > port->h - 1) {
        clip = (i16)(bottom - port->h + 1 > rows ? rows : bottom - port->h + 1);
        rows = (i8)(rows - clip);
    }
    if (rows <= 0)
        return 0;
    dst = buffer_pointer(port, x, y);
    d = dst;
    cursor = x;
    for (p = (const u8 *)buf; ; p++) {
        i8 code = (i8)(*p - 1);
        u8 w;
        if (code < 0)
            break;
        w = font[2 + (u8)code];
        if (w) {
            const u8 *g;
            char *rowp = d;
            u8 r;
            cursor = (i16)(cursor + w);
            if ((u16)cursor > (u16)port->w)
                break;
            g = font + *(const u16 *)(font + 0x82 + 2 * (u8)code);
            if (skip)
                g += (u16)skip * ((u16)((w - 1) >> 2) + 1);
            for (r = (u8)rows; r; r--) {
                u8 left = w, k = 4, bits = *g++;
                char *q = rowp;
                for (;;) {
                    u8 c = cols.c[bits >> 6];
                    bits = (u8)(bits << 2);
                    if (c != 0xff)
                        *q = (char)c;
                    q++;
                    if (--left == 0)
                        break;
                    if (--k == 0) {
                        bits = *g++;
                        k = 4;
                    }
                }
                rowp += port->w;
            }
            d += w + gap;
            cursor = (i16)(cursor + gap);
        }
    }
    return (i16)(x + (d - dst));
}
