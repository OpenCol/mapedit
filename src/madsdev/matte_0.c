/* From mapedit-decomp src/madsdev/matte_0 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "records.h"
#include "screen.h"
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

