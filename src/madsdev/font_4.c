/* Imported from mapedit-decomp src/madsdev/font_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE font_4: MAPEDIT.EXE segment 0d6c (font_4.c), file 0xecc2-0xed0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn font_string_width 0x0ecc2
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 *  font_string_width is the whole 72-byte module.)
 *
 * font_string_width(spacing, font, s) (_fastcall) sums the widths of the characters of a
 * far string: each character's width from the font's table (byte 2 + code - 1), plus
 * `spacing` after every character but the last that has a width.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { char hdr[2]; u8 width[1]; } Font;
/* font_string_width -- 0d6c:0002, 72 bytes */
// MSC: #pragma optimize("elg", on)
i16 font_string_width(i16 spacing, Font *font, char *s)
{
    i16 w;
    i16 cw;

    w = 0;
    while (*s) {
        cw = font->width[*s++ - 1];
        if (cw > 0 && *s)
            cw += spacing;
        w += cw;
    }
    return w;
}

