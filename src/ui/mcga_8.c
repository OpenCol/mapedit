/* From mapedit-decomp src/madsdev/mcga_8 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
/* MODULE mcga_8: MAPEDIT.EXE segment 105c (mcga_8.c), file 0x11bca-0x11c1a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_getpal  0x11bca
 *
 * Near twin: `ovl30_78548` read_palette (pascal there; _fastcall here).
 *
 * READING THE WHOLE PALETTE. mcga_getpal(pal) (_fastcall) reads the 256 DAC entries into
 * the far buffer, 64 bytes per vertical retrace with interrupts off and two I/O-delay
 * jumps per byte; mcga_palette_update is raised while it runs.
 */


/* mcga_getpal -- 105c:000a, 80 bytes */
/* PORT: mcga_getpal is in port/ (inline assembly) */


