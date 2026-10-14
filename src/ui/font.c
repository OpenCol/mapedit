/* font.c -- fonts: loading them (.FF files), their colours, writing and measuring text.
 *
 * A font is a whole .FF file held in memory:
 *   byte 0     the height of every glyph, in pixels;
 *   from 2     one width byte per character, indexed by code - 1 (codes 1..80h);
 *   from 82h   one word per character, the offset of its glyph from the start of the font.
 * A glyph is `height` rows of `width` pixels, two bits a pixel, four pixels to a byte, high
 * bits first, each row starting on a new byte. The two bits pick one of the four colours in
 * font_colors; a colour of FFh is transparent (the destination is left as it is).
 *
 * The editor loads its two fonts once at startup (font_load) into font_inter and menu_font,
 * sets the colours with font_set_colors, measures strings with font_string_width and draws
 * them with font_write.
 */
#include <string.h>

#include "msc.h"
#include "assets.h"
#include "buffer.h"
#include "error.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#include "w16types.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */

/* Loads a whole font file into a new memory block. `name` gets ".FF" when it has no
 * extension; a leading '*' is skipped and the next eight characters tag the block
 * (mem_get_name). Returns the block, or 0 when the file cannot be opened, the block cannot be
 * allocated or the read fails (a block already taken is freed then). Sets
 * mem_last_alloc_loader to 0Fh, the number memory errors are reported under.
 */
void *font_load(char *name)
{
    FileIO fh;
    char path[80];
    char tag[20];
    void *result;
    i32 size;
    void *buf;
    char *p;

    buf = 0;
    result = 0;
    mem_last_alloc_loader = 0x0f;
    fh.msc_open = 0;
    msc_strcpy(path, name);
    if (msc_strchr(path, '.') == 0)
        msc_strcat(path, ".FF");
    p = path;
    if (*p == '*')
        p = path + 1;
    msc_strncpy(tag, p, 8);
    if (loader_open("rb", -1, &fh, path) == 0) {
        size = fh.ent[0].size;   /* Port: the file's (first entry's) unpacked size */
        buf = mem_get_name(size, tag);
        if (buf != 0) {
            if (loader_read(size, buf, 1L, &fh) != 0)
                result = buf;
        }
    }
    if (buf != 0 && result == 0)
        mem_free(buf);
    if (fh.msc_open)
        loader_close(&fh);
    return result;
}

/* The interface font, loaded from FONTINTR. The other font, menu_font (FONTTINY), is in
 * theme.c. */
void *font_inter = 0;                 /* the interface font */

/* Returns the width in pixels of string s: the sum of its characters' widths, plus `spacing`
 * after each character that has a width and is not the last. Returns 0 for an empty string.
 * Port: the original sign-extends the character, so a code from 80h up reads before the
 * width table; the port reads the code as unsigned. */
i16 font_string_width(i16 spacing, DosFont *font_, char *s)
{
    const u8 *font = (const u8 *)font_;
    i16 total = 0;
    const u8 *p = (const u8 *)s;
    if (!*p)
        return 0;
    do {
        u8 w = font[2 + (u8)(*p - 1)];
        i16 c = w;
        p++;
        if (c > 0 && *p != 0)
            c = (i16)(c + spacing);
        total = (i16)(total + c);
    } while (*p);
    return total;
}

/* The four colours a glyph's two-bit pixels map to (FFh = transparent): by default a
 * transparent background, white, grey and dark grey. */
u8 font_colors[4] = { 0xff, 0x0f, 0x07, 0x08 };

/* Sets the four text colours font_write maps a glyph's pixel values 0..3 to. Each is stored
 * as a byte, so -1 (FFh) makes that value transparent, usually the background in a. */
void font_set_colors(i16 a, i16 b, i16 c, i16 d)
{
    font_colors[0] = a;
    font_colors[1] = b;
    font_colors[2] = c;
    font_colors[3] = d;
}

/* Draws string s into port with its top-left corner at (x, y), `gap` pixels between
 * characters, and returns the x after the last character drawn plus its gap (0 when no row
 * of the font is inside the port).
 *
 * The string ends at a 0, and also at any code from 81h up (the original decrements the code
 * and stops on the sign). A character that would pass the port's right edge stops the
 * string; an empty-width character draws nothing and adds no gap. Rows above the port are
 * skipped in the glyph, rows below it clipped; there is no clipping at the left edge.
 */
i16 font_write(DosPort *port, i16 x, i16 y, DosFont *font_, char *s, i16 gap)
{
    const u8 *font = (const u8 *)font_;
    char buf[80];
    u8 cols[4];
    i8 rows;
    i16 bottom, skip = 0, clip, cursor;
    char *dst, *d;
    const u8 *p;
    u16 n = 0;
    memcpy(cols, font_colors, sizeof cols);

    while (n < sizeof buf - 1 && s[n]) {        /* Port: the original copies without a limit */
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
                    u8 c = cols[bits >> 6];
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
