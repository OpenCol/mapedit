/* From mapedit-decomp src/madsdev/timer_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
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

/* TIMER_SET_RATE -- 105b:0006, 20 bytes */
/* PORT: TIMER_SET_RATE is in port/ (inline assembly) */


