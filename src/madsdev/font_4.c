/* From mapedit-decomp src/madsdev/font_4 (f55aff8), now maintained here: docs/origin.md. */
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


extern void *font_main;               /* DS:0x3a9a (font_1.c); only its SEG (DGROUP) is used */
/* font_string_width -- 0d6c:0002, 72 bytes */
/* PORT: font_string_width is in port/ (inline assembly) */
i16 font_string_width(i16 spacing, void *font, char *s);


