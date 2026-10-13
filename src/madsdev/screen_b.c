/* From mapedit-decomp src/madsdev/screen_b (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "machine.h"
#include "records.h"
#include "screen.h"
#include "units.h"
/* MODULE screen_b: MAPEDIT.EXE segment 0c3c (SCREEN_B.C), file 0xd9ca-0xd9ea, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn screen_dominant_mode 0x0d9ca
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b8b).
 *
 * THE BIOS VIDEO EQUIPMENT BITS (root code, frame 0b8b). screen_dominant_mode(mode) (_fastcall)
 * sets bits 4-5 of 0040:0010 to monochrome (mode 7) or 80-column colour.
 */

/* screen_dominant_mode -- 0c3c:000a, 32 bytes */
void screen_dominant_mode(i16 mode)
{
    *(u8 *)(msc_bda + 0x0010) &= 0xcf;
    if (mode == 7)
        *(u8 *)(msc_bda + 0x0010) |= 0x30;
    else
        *(u8 *)(msc_bda + 0x0010) |= 0x20;
}

