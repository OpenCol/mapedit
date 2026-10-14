/* video.c -- the mode 13h screen: setting the mode, copying work buffers to the screen.
 *
 * The screen is the host's framebuffer (platform/host.h): 320 x 200 palette indices, row-major,
 * where the original had the VGA's memory. Writing it marks it changed; the host shows it at
 * the next 70 Hz frame, as the VGA did.
 */
#include "msc.h"
#include "error.h"
#include "pack.h"
#include "map.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "mapedit.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#include "video.h"
#include "host.h"

/* VIDEO_ADDRESS -- the address of screen pixel (x, y). The offset is computed in 16 bits. */
char *VIDEO_ADDRESS(i16 x, i16 y)
{
    return (char *)host_vram() + (u16)((u16)y * 320u + (u16)x);
}

/* video_init -- record the video mode in video_mode and, when set is nonzero, set it, which
 * clears the screen as the BIOS mode switch did. Mode 13h is the only graphics mode the port
 * has, so setting a mode does nothing else.
 */
void video_init(i16 mode, i16 set)
{
    video_mode = mode;
    if (set) {
        u8 *v = host_vram();
        u16 i;
        for (i = 0; i < 64000u; i++)
            v[i] = 0;
        host_vram_touched();
    }
}

/* video_update -- copy w x h pixels at (sx, sy) of a work buffer to the screen at (dx, dy).
 * Rows advance by the buffer's width and the screen's 320. Nothing is clipped.
 */
void video_update(DosPort *port, i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h)
{
    const char *s = port->bits + ((u32)(u16)sy * (u16)port->w + (u16)sx);
    char *d = (char *)host_vram() + (u16)(320u * (u16)dy + (u16)dx);
    u16 r, i;
    for (r = (u16)h; r != 0; r--) {
        for (i = 0; i < (u16)w; i++)
            d[i] = s[i];
        s += port->w;
        d += 320;
    }
    host_vram_touched();
}

i16 video_mode;                           /* the mode last passed to video_init */

/* MCGA_SHAKE -- one step of a screen shake: counts MCGA_SHAKES down to 0. The original also
 * moved the CRTC's start address; the editor never starts a shake, so the port only counts.
 */
void MCGA_SHAKE(void)
{
    if (MCGA_SHAKES)
        MCGA_SHAKES--;
}

i16 MCGA_SHAKES = 0;                             /* steps of screen shake left */
