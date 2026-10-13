/* From mapedit-decomp src/madsdev/mcga_7 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "mcga.h"
/* MODULE mcga_7: MAPEDIT.EXE segment 0d70 (mcga_7.c), file 0xed0a-0xed5e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_setpal  0x0ed0a
 *
 * No VICEROY twin: reconstructed from the shipped bytes (it is set_palette_range with
 * first = 0 and count = 256, plus two I/O-delay jumps per byte).
 *
 * WRITING THE WHOLE PALETTE. mcga_setpal(pal) (_fastcall) writes all 256 DAC entries from
 * the far palette, at most mcga_retrace_max_bytes per vertical retrace with interrupts
 * off; mcga_palette_update is raised while it runs. This module also defines the mcga
 * retrace globals (DS:0x3aae-0x3ab8).
 */


i16 mcga_retrace_computed = 0;                   /* DS:0x3aae (defined here, the six in address order) */
u16 mcga_retrace_ticks = 0;                 /* DS:0x3ab0 */
i16 mcga_retrace_max_colors = 256;               /* DS:0x3ab2 */
i16 mcga_retrace_max_bytes = 768;                /* DS:0x3ab4 */
i16 mcga_palette_update = 0;                     /* DS:0x3ab6 */
i16 mcga_palette_fast = 1;                       /* DS:0x3ab8 */
/* mcga_setpal -- 0d70:000a, 84 bytes */
/* PORT: mcga_setpal is in port/ (inline assembly) */


