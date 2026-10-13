/* Imported from mapedit-decomp src/madsdev/font_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 c[4]; } Colours;

extern Colours font_colors;             /* DS:0x3aaa */
extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
/* font_write -- 0d53:0008, 366 bytes */
/* PORT: font_write is in port/ (inline assembly) */
i16 font_write(DosPort *port, i16 x, i16 y, u8 *font, char *s, i16 gap);


/* The module's data (DS:0x3aaa): the text colours, after the last function (so no function's
 * symbol count moves). The five font pointers before it (DS:0x3a96-0x3aa9) are font_1.c's. */
Colours font_colors = { { 0xff, 0x0f, 0x07, 0x08 } };        /* DS:0x3aaa */

