/* host.h -- the PC the 1994 editor ran on, as the port provides it.
 *
 * Everything the editor's machine layer reached through hardware or the BIOS comes from
 * here: VGA mode 13h (a 320x200 framebuffer of palette indices and the 6-bit DAC), the
 * keyboard (the BIOS queue of scan code / ASCII pairs and the shift flags), the mouse (the
 * INT 33h position and buttons, in screen pixels) and the timer. The SDL3 implementation is
 * host_sdl.c. The C stand-ins for the asm modules (video, mouse, keys, timer) are built on
 * this interface and nothing else, so a test can drive them headless.
 */
#ifndef MAPEDIT_HOST_H
#define MAPEDIT_HOST_H

#include <stdint.h>

#define HOST_W 320
#define HOST_H 200

/* BIOS keyboard shift flags (0040:0017), the bits the editor tests */
#define HOST_SHIFT_RIGHT 0x01
#define HOST_SHIFT_LEFT  0x02
#define HOST_CTRL        0x04
#define HOST_ALT         0x08

/* INT 33h button bits */
#define HOST_BUTTON_LEFT  0x01
#define HOST_BUTTON_RIGHT 0x02

typedef struct HostOptions {
    const char *title;
    int scale;          /* window size as a multiple of 320x200 (or 320x240 with aspect) */
    int aspect;         /* nonzero: show 320x200 at 4:3, as a 1994 monitor did */
    int headless;       /* nonzero: no window (tests); the framebuffer still works */
} HostOptions;

int  host_init(const HostOptions *opt);    /* 0 on success */
void host_quit(void);

/* video: mode 13h */
uint8_t *host_vram(void);                  /* HOST_W * HOST_H palette indices, row-major */
void host_set_dac(int first, int count, const uint8_t *rgb6);   /* 3 bytes each, 0..63 */
void host_get_dac(int first, int count, uint8_t *rgb6);
void host_present(void);                   /* show the framebuffer through the DAC */
int  host_vretrace(void);                  /* nonzero during vertical retrace (status port 3DAh bit 3) */
const uint32_t *host_rgb(void);            /* the last presented frame as 0x00RRGGBB, for tests */

/* events: call often; it runs the OS's event loop and fills the queues below, and shows a
   changed framebuffer at most 70 times a second (the VGA refresh rate) */
void host_pump(void);
int  host_quit_requested(void);
void host_vram_touched(void);              /* the framebuffer changed: show it at the next frame */

/* the mouse driver's event handler (INT 33h function 14h): called from host_pump on every
   move and button change, with screen pixels and button bits */
typedef void (*HostMouseHandler)(int x, int y, int buttons);
void host_set_mouse_handler(HostMouseHandler fn);

/* a keyboard interrupt hook (INT 9): sees each key before the BIOS queue does. scan is the
   set-1 scan code, keypad nonzero for the numeric keypad (not the grey keys, which sent E0
   first). A nonzero return swallows the key. */
typedef int (*HostKeyHook)(int scan, int down, int keypad);
void host_set_key_hook(HostKeyHook fn);

/* keyboard: the BIOS queue (INT 16h) */
int      host_key_ready(void);
uint16_t host_key_get(void);               /* scan code << 8 | ASCII; 0 if none */
void     host_key_flush(void);
int      host_shift_flags(void);
uint16_t host_key_translate(int sdl_scancode, int shift, int ctrl, int alt);

/* mouse: INT 33h */
void host_mouse(int *x, int *y, int *buttons);
void host_mouse_warp(int x, int y);

/* time */
uint32_t host_ms(void);                    /* milliseconds since host_init */
void     host_virtual_time(int on);        /* tests: time moves only by host_advance */
void     host_advance(uint32_t ms);
void     host_sleep(uint32_t ms);

#endif
