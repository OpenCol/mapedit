/* screen.c -- the screens the editor draws into: the work, original and video ports. */
#include "msc.h"
#include "font.h"
#include "loader.h"
#include "machine.h"
#include "records.h"
#include "screen.h"
#include "units.h"
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

/* screen_dominant_mode -- 0c3c:000a, 32 bytes */
void screen_dominant_mode(i16 mode)
{
    *(u8 *)(msc_bda + 0x0010) &= 0xcf;
    if (mode == 7)
        *(u8 *)(msc_bda + 0x0010) |= 0x30;
    else
        *(u8 *)(msc_bda + 0x0010) |= 0x20;
}

/* MODULE matte_0: MAPEDIT.EXE data only (matte_0.c), data at DS:0x3aec.
 *
 * No code: the seven screen ports (madsdev matte_0.c). A port is its height, its width and a
 * far pointer to its pixels; scr_video is the MCGA frame buffer (200 x 320 at A000:0000), the
 * others are filled in at run time. The 56 bytes land at DS:0x3aec-0x3b24, word-aligned
 * after pal_8.c's "rb" (DS:0x3ae8-0x3aeb), before error_1.c's literals.
 */


DosPort scr_main = { 0, 0, 0 };                                 /* DS:0x3aec */
DosPort scr_work = { 0, 0, 0 };                                 /* DS:0x3af4 */
DosPort scr_orig = { 0, 0, 0 };                                 /* DS:0x3afc */
DosPort scr_depth = { 0, 0, 0 };                                /* DS:0x3b04 */
DosPort scr_walk = { 0, 0, 0 };                                 /* DS:0x3b0c */
DosPort scr_special = { 0, 0, 0 };                              /* DS:0x3b14 */
DosPort scr_video = { 200, 320, (void *)(msc_vga + 0x0000) };      /* DS:0x3b1c */
