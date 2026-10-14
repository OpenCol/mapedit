/* screen.c -- the screens the editor draws into: the work, original and video ports.
 *
 * Each screen is a DosPort (buffer.c): a height, a width and a pointer to row-major palette
 * indices. scr_video is the 320 x 200 mode 13h frame buffer; scr_orig and scr_work start empty
 * and are allocated at startup, both 320 x 200: the map view is rendered into scr_orig,
 * regions of it are copied into scr_work, where the cursor, menus and popups are drawn over
 * them, and scr_work goes to the screen.
 */
#include "msc.h"
#include "font.h"
#include "loader.h"
#include "machine.h"
#include "records.h"
#include "screen.h"
#include "units.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as in 1994; file formats rely on it */

/* screen_dominant_mode -- set the BIOS equipment flags' initial video mode (bits 4-5 of the
 * equipment word): monochrome for mode 7, 80-column colour for anything else. The port's BIOS
 * data area is a plain array, so this changes nothing on screen.
 */
void screen_dominant_mode(i16 mode)
{
    *(u8 *)(msc_bda + 0x0010) &= 0xcf;
    if (mode == 7)
        *(u8 *)(msc_bda + 0x0010) |= 0x30;
    else
        *(u8 *)(msc_bda + 0x0010) |= 0x20;
}

DosPort scr_work = { 0, 0, 0 };   /* what goes to the screen: the map plus overlays */
DosPort scr_orig = { 0, 0, 0 };   /* the map view as rendered, without overlays */
DosPort scr_video = { 200, 320, (void *)(msc_vga + 0x0000) };   /* the screen itself */
