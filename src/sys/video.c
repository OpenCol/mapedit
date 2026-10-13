/* video.c -- video_1.ASM and video_2.ASM: the mode 13h screen.
 *
 * The screen is the host's framebuffer (platform/host.h), 320 x 200 palette indices at what
 * was A000:0000. Writing it marks it changed; the host shows it at the next 70 Hz frame, as
 * the VGA did.
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

/* VIDEO_ADDRESS -- 1114:0002: the screen address of (x, y), x in CX and y in DX */
char *VIDEO_ADDRESS(i16 x, i16 y)
{
    return (char *)host_vram() + (u16)((u16)y * 320u + (u16)x);
}

/* video_init -- 0ec5:0008: remember the mode; set it through the BIOS (INT 10h, AH=0) when
   asked. Mode 13h is the only graphics mode the port has; setting a mode clears the screen,
   as the BIOS did. */
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

/* video_update -- 0ec5:0024: copy w x h pixels at (sx, sy) of a work buffer to the screen at
   (dx, dy). The work buffer is a DosPort; rows advance by its width and the screen's 320. */
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

/* mapedit.obj's uninitialised data: communals, placed by the linker in BSS (from DS:0x4999).
 * Sizes are the distance to the next public; a global nothing in the editor reads keeps a
 * plain array of that size. A definition after the last function moves no symbol position. */
i16 video_mode;                           /* DS:0x49c6 */

/* MCGA_SHAKE -- 12ad:000c: one step of a screen shake (the CRTC's start address, which the
   editor never shakes); counts the shake down */
void MCGA_SHAKE(void)
{
    if (MCGA_SHAKES)
        MCGA_SHAKES--;
}

i16 MCGA_SHAKES = 0;                             /* DS:0x450e: shakes left (defined here) */
