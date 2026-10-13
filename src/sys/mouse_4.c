/* From mapedit-decomp src/madsdev/mouse_4 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "popup.h"
#include "loader.h"
#include "mouse.h"
#include "records.h"
#include "screen.h"
#include "units.h"
#include "video.h"
/* MODULE mouse_4: MAPEDIT.EXE segment 0c34 (mouse_4.c), file 0xd94c-0xd9ca, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mouse_video_init   0x0d94c
 * fn mouse_video_update 0x0d984
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b70).
 *
 * SHOWING THE SCREEN PORT (root code, frame 0b70). The calls go to the assembly mouse/video
 * module at 0a58 and the copier video_update (0d11:001c).
 *   - mouse_video_init hands the screen port's pixels and size to the mouse code.
 *   - mouse_video_update(sx, sy, dx, dy, w, h) (_fastcall) hides the mouse, copies the rectangle
 *     from the screen port to video memory, and shows the mouse again if it was visible.
 */



/* mouse_video_init -- 0c34:000c, 56 bytes */
void mouse_video_init(void)
{
    mouse_set_work_buffer(scr_work.bits, scr_work.w);
    mouse_set_view_port_loc(0, 0, scr_work.w - 1, scr_work.h - 1);
    mouse_set_view_port(0, 0);
}
/* mouse_video_update -- 0c34:0044, 70 bytes */
void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h)
{
    i16 vis;

    mouse_freeze();
    vis = mouse_refresh_view_port();
    video_update(&scr_work, sx, sy, dx, dy, w, h);
    if (vis)
        mouse_refresh_done();
    mouse_thaw();
}

