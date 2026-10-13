/* Imported from mapedit-decomp src/madsdev/buffer_6 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_6: MAPEDIT.EXE segment 0c67 (buffer_6.c), file 0xdc70-0xdd64, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_copy_2 0x0dc70
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0baa).
 *
 * COPYING A RECTANGLE BETWEEN PORTS (root code, frame 0baa). buffer_rect_copy_2(sx, sy, dx, src,
 * dst, dy, w, h) (_fastcall) is port_blit (root_0b8f.c) with separate source and destination
 * corners. The rows are copied in inline assembly: word moves, an odd
 * byte, and a 32 KB segment step on each side.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
extern char *mem_normalize(char *p);
/* buffer_rect_copy_2 -- 0c67:0000, 244 bytes */
/* PORT: buffer_rect_copy_2 is in port/ (inline assembly) */
i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h);


