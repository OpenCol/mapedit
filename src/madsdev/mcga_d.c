/* From mapedit-decomp src/madsdev/mcga_d (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE mcga_d: MAPEDIT.EXE segment 12ad (mcga_d.c), file 0x140dc-0x14100, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn MCGA_SHAKE   0x140dc
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * SCREEN SHAKE. MCGA_SHAKE() steps a linear-congruential seed (x * 5 + 1), keeps its top
 * two bits of the high byte as a 0..0x3ff display-start offset, and writes it to CRTC
 * register 0x0d (start address low, with the high byte in ah going to 0x0e through the
 * word out). When MCGA_SHAKES counts down to zero the high byte is cleared, so the
 * last shake lands within the first 256 bytes. Entirely inline assembly.
 */


i16 MCGA_SHAKES = 0;                             /* DS:0x450e: shakes left (defined here) */
static i16 shake_seed = 1234;                    /* DS:0x4510: module static (no public) */
/* MCGA_SHAKE -- 12ad:000c, 36 bytes */
/* PORT: MCGA_SHAKE is in port/ (inline assembly) */
void MCGA_SHAKE(void);


