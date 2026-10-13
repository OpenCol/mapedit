/* Imported from mapedit-decomp src/madsdev/screen_b by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* screen_dominant_mode -- 0c3c:000a, 32 bytes */
// MSC: #pragma optimize("elg", on)
void screen_dominant_mode(i16 mode)
{
    *(u8 *)(msc_bda + 0x0010) &= 0xcf;
    if (mode == 7)
        *(u8 *)(msc_bda + 0x0010) |= 0x30;
    else
        *(u8 *)(msc_bda + 0x0010) |= 0x20;
}

