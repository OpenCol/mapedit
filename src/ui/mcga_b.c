/* From mapedit-decomp src/madsdev/mcga_b (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
/* MODULE mcga_b: MAPEDIT.EXE segment 1061 (mcga_b.c), file 0x11c1a-0x11c96, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_retrace      0x11c1a
 * fn mcga_setpal_range 0x11c2e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c2e).
 *
 * THE VGA PALETTE (madsdev mcga_b.c).
 *   - mcga_retrace waits for the start of a vertical retrace.
 *   - mcga_setpal_range(first, count, pal) (_fastcall) writes `count` DAC entries from
 *     `first`, at most mcga_retrace_max_bytes per retrace with interrupts off.
 *     mcga_palette_update is -1 while it runs (VICEROY's set_palette_range raises 1).
 */


/* mcga_retrace -- 1061:000a, 20 bytes */
/* PORT: mcga_retrace is in port/ (inline assembly) */

/* mcga_setpal_range -- 1061:001e, 104 bytes */
/* PORT: mcga_setpal_range is in port/ (inline assembly) */


