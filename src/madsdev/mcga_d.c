/* Imported from mapedit-decomp src/madsdev/mcga_d by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i16 MCGA_SHAKES;                          /* DS:0x450e: shakes left */
extern i16 shake_seed;                           /* DS:0x4510: module static (no public) */
/* MCGA_SHAKE -- 12ad:000c, 36 bytes */
/* PORT: MCGA_SHAKE is in port/ (inline assembly) */
void MCGA_SHAKE(void);


