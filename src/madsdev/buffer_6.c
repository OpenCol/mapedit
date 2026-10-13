/* From mapedit-decomp src/madsdev/buffer_6 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
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



/* buffer_rect_copy_2 -- 0c67:0000, 244 bytes */
/* PORT: buffer_rect_copy_2 is in port/ (inline assembly) */


