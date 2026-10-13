/* Imported from mapedit-decomp src/madsdev/mouse_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;


extern DosPort scr_work;            /* DS:0x2da8 */
extern void mouse_set_work_buffer(char *bits, i16 w);
extern void mouse_set_view_port_loc(i16 x0, i16 y0, i16 x1, i16 y1);
extern void mouse_set_view_port(i16 a, i16 b);
extern void mouse_freeze(void);
extern i16 mouse_refresh_view_port(void);
extern void mouse_refresh_done(void);
extern void mouse_thaw(void);
extern void video_update(DosPort *port, i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);
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

