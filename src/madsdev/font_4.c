/* Imported from mapedit-decomp src/madsdev/font_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern char *font_main;               /* only its SEG (DGROUP) is used */
/* font_string_width -- 0d6c:0002, 72 bytes */
/* PORT: font_string_width is in port/ (inline assembly) */
i16 font_string_width(i16 spacing, void *font, char *s);


