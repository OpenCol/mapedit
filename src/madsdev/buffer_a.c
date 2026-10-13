/* Imported from mapedit-decomp src/madsdev/buffer_a by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_a: MAPEDIT.EXE segment 0c80 (buffer_a.c), file 0xde00-0xde6c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_vline 0x0de00
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bc3).
 *
 * A VERTICAL LINE (root code, frame 0bc3). buffer_vline(x, y0, y1, port, color) (_fastcall) clips
 * to the port in C, then draws in inline assembly.
 */

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; u16 off; u16 seg; } DosPort;
/* buffer_vline -- 0c80:0000, 108 bytes */
/* PORT: buffer_vline is in port/ (inline assembly) */
void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color);


