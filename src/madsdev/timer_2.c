/* Imported from mapedit-decomp src/madsdev/timer_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE timer_2: MAPEDIT.EXE segment 105b (TIMER_2.C), file 0x11bb6-0x11bca, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn TIMER_SET_RATE 0x11bb6
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c10).
 *
 * THE TIMER RATE (root code, frame 0c10). TIMER_SET_RATE(divisor) programs PIT channel 0 in
 * mode 3 (control word 0x36) with the given divisor, interrupts off. Inline assembly.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* TIMER_SET_RATE -- 105b:0006, 20 bytes */
/* PORT: TIMER_SET_RATE is in port/ (inline assembly) */
void TIMER_SET_RATE(u16 divisor);


