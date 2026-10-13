/* Imported from mapedit-decomp src/madsdev/buffer_5 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_5: MAPEDIT.EXE segment 0c5b (BUFFER_5.C), file 0xdbb4-0xdc70, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_fill 0x0dbb4
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b9e).
 *
 * FILLING A RECTANGLE (root code, frame 0b9e). buffer_rect_fill(x, y, w, port, h, color)
 * (_fastcall) clips to the port (buffer_conform), then fills the rows in inline assembly: word
 * stores, an odd byte, and a 32 KB segment step. It returns whether the port had pixels;
 * after a clip that leaves nothing, the return value is uninitialised (shipped behaviour).
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern i16 buffer_conform(DosPort *port, i16 *x, i16 *y, i16 *w, i16 *h);
extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
extern char *mem_normalize(char *p);
/* buffer_rect_fill -- 0c5b:0004, 188 bytes */
/* PORT: buffer_rect_fill is in port/ (inline assembly) */
i16 buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);


