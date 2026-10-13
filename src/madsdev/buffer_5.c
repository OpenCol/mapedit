/* From mapedit-decomp src/madsdev/buffer_5 (f55aff8), now maintained here: docs/origin.md. */
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



extern i16 buffer_conform(DosPort *port, i16 *x, i16 *y, i16 *w, i16 *h);
extern char *buffer_pointer(DosPort *port, i16 x, i16 y);
extern char *mem_normalize(char *p);
/* buffer_rect_fill -- 0c5b:0004, 188 bytes */
/* PORT: buffer_rect_fill is in port/ (inline assembly) */
i16 buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);


