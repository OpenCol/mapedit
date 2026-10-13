/* From mapedit-decomp src/madsdev/mcga_b (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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


extern i16 mcga_retrace_max_bytes;               /* DS:0x3ab4: DAC bytes per retrace */
extern i16 mcga_palette_update;                  /* DS:0x3ab6: the palette is being written */
/* mcga_retrace -- 1061:000a, 20 bytes */
/* PORT: mcga_retrace is in port/ (inline assembly) */
void mcga_retrace(void);

/* mcga_setpal_range -- 1061:001e, 104 bytes */
/* PORT: mcga_setpal_range is in port/ (inline assembly) */
void mcga_setpal_range(i16 first, i16 count, char *pal);


