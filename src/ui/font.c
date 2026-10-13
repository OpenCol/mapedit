/* font.c -- fonts: loading them (.FF files), their colours, writing and measuring text. */
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
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE font_1: MAPEDIT.EXE segment 0d43 (font_1.c), file 0xea3a-0xeb38, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_load    0x0ea3a
 *
 * Near twin: `ovl27_76c70` ff_load (0.99): the same source with the library
 * names. This module also defines the five font pointers (DS:0x3a96-0x3aa6).
 *
 * LOADING A WHOLE .FF FONT FILE INTO FAR MEMORY. font_load(name) (_fastcall, name in bx):
 *   1. Copy the name and add ".FF" when it has no extension.
 *   2. Skip a leading '*' and take the first 8 characters as the allocation's tag.
 *   3. Open the file "rb", allocate a block of the file's size under that tag
 *      (mem_get_name), and read the whole file into it (loader_read).
 *   4. On a failed read, free the block. Close the file if it was opened.
 * It returns the block, or 0. It sets mem_last_alloc_loader to 0x0f first: the module
 * number a file error is reported under.
 *
 * Global register allocation plus the global optimiser (`#pragma optimize("eg")`), as the twin.
 */

/* font_load -- 0d43:000a, 254 bytes */
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
        size = fh.ent[0].size;   /* PORT */
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

/* The five loaded fonts (CodeView: font_1.c), defined after the last function: they land after
 * font_load's ".FF" and "rb" at DS:0x3a96-0x3aa9. */
void *font_inter = 0;                 /* DS:0x3a96 */
void *font_main = 0;                  /* DS:0x3a9a */
void *font_conv = 0;                  /* DS:0x3a9e */
void *font_menu = 0;                  /* DS:0x3aa2 */
void *font_misc = 0;                  /* DS:0x3aa6 */

/* font_string_width -- 0d6c:0002: the glyph widths of s, plus spacing after each glyph that
   has a width and is not the last character. (The original sign-extends the character, so a
   code from 80h up reads before the width table; the port reads it as unsigned.) */
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

/* MODULE font_2: MAPEDIT.EXE segment 0d53 (font_2.c), file 0xeb38-0xeca6, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_write   0x0eb38
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c11).
 *
 * DRAWING TEXT (root code, frame 0c11; "elg"). font_write(port, x, y, font, s, gap)
 * (_fastcall) draws s at (x, y) in the font, 2 bits per pixel mapped through the four
 * colours text_colors set (0xff = transparent), clipped at the port's top and bottom and
 * stopped at its right edge, gap pixels between characters. It returns the x after the
 * last character drawn (0 when nothing fits). The glyph loop is inline assembly.
 */



/* font_write -- 0d53:0008, 366 bytes */
/* PORT: font_write is in port/ (inline assembly) */


/* The module's data (DS:0x3aaa): the text colours, after the last function (so no function's
 * symbol count moves). The five font pointers before it (DS:0x3a96-0x3aa9) are font_1.c's. */
u8 font_colors[4] = { 0xff, 0x0f, 0x07, 0x08 };        /* DS:0x3aaa */

/* MODULE font_3: MAPEDIT.EXE segment 0d6a (FONT_3.C), file 0xeca6-0xecc2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_set_colors 0x0eca6
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c28).
 *
 * TEXT COLOURS (root code, frame 0c28; "elg"). font_set_colors(a, b, c, d) (_fastcall) stores the four
 * colour bytes draw_text reads (DS:0x269e-0x26a1); callers pass -1 for "unchanged" in a.
 */


/* font_set_colors -- 0d6a:0006, 28 bytes */
void font_set_colors(i16 a, i16 b, i16 c, i16 d)
{
    font_colors[0] = a;
    font_colors[1] = b;
    font_colors[2] = c;
    font_colors[3] = d;
}

/* MODULE font_4: MAPEDIT.EXE segment 0d6c (font_4.c), file 0xecc2-0xed0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_string_width 0x0ecc2
 *
 * Twin: `root_0c2a` text_width (VICEROY.EXE 0xe6a6), the same bytes.
 *
 *  font_string_width is the whole 72-byte module.)
 *
 * font_string_width(spacing, font, s) (_fastcall) sums the widths of the characters of a
 * far string: each character's width from the font's table (byte 2 + code - 1), plus
 * `spacing` after every character but the last that has a width. The loop is inline
 * assembly; it restores DS from DGROUP (a SEG fixup) at the end.
 */


/* font_string_width -- 0d6c:0002, 72 bytes */
/* PORT: font_string_width is in port/ (inline assembly) */

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
