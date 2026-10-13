/* mcga.c -- the palette routines of mcga_7/8/b/c/d.c whose port access was inline assembly.
 *
 * The VGA's DAC is the host's (host_set_dac). The originals write it a chunk at a time, each
 * chunk after the start of a vertical retrace, so that colour changes never tear: a palette
 * of 768 bytes takes as many frames as it has chunks. The port waits for the next 70 Hz frame
 * the same way, so fades last as long as they did. How much fits in one retrace was measured
 * on the machine (mcga_compute_retrace_parameters); the port reports a fast one, as the
 * measurement would on anything later than a 386: every colour in one retrace.
 */
#include "msc.h"
#include "host.h"

void timer_sync(void);

extern i16 mcga_retrace_max_bytes, mcga_retrace_max_colors, mcga_retrace_computed;
extern i16 mcga_palette_update, mcga_palette_fast;
extern i16 MCGA_SHAKES;

/* mcga_retrace -- 1061:000a: wait for the next vertical retrace to begin */
void mcga_retrace(void)
{
    u32 frame = host_ms() * 70u / 1000u;
    while (host_ms() * 70u / 1000u == frame) {
        host_pump();
        timer_sync();
        host_sleep(1);
    }
}

/* the chunked DAC write of mcga_setpal and mcga_setpal_range */
static void dac_write(i16 first, const u8 *rgb, u16 bytes, u16 chunk)
{
    u16 colour = (u16)first * 3;
    if (chunk == 0)
        chunk = 0x300;
    while (bytes) {
        u16 n = bytes > chunk ? chunk : bytes, i;
        mcga_retrace();
        for (i = 0; i < n; i++, colour++) {
            u8 c[3];
            host_get_dac(colour / 3, 1, c);
            c[colour % 3] = rgb[i];
            host_set_dac(colour / 3, 1, c);
        }
        rgb += n;
        bytes -= n;
    }
    host_vram_touched();
}

/* mcga_setpal -- 0d70:000a: all 256 colours, 768 bytes of 6-bit components */
void mcga_setpal(char *pal)
{
    mcga_palette_update = 1;
    dac_write(0, (const u8 *)pal, 0x300, (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* mcga_setpal_range -- 1061:001e: count colours from first, read at pal + first * 3 */
void mcga_setpal_range(i16 first, i16 count, char *pal)
{
    mcga_palette_update = -1;
    dac_write(first, (const u8 *)pal + first * 3, (u16)(count * 3), (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* mcga_getpal -- 105c:000a: the DAC's 768 bytes, read 64 at a time, one chunk per retrace */
void mcga_getpal(char *pal)
{
    u16 i;
    mcga_palette_update = 1;
    for (i = 0; i < 0x300 / 0x40; i++)
        mcga_retrace();
    host_get_dac(0, 256, (u8 *)pal);
    mcga_palette_update = 0;
}

/* mcga_time_palette_swap -- 0d75:000e: write a range of colours and return how many PIT
   ticks it took. The port's DAC takes none. */
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal)
{
    host_set_dac(first, count, (const u8 *)pal + first * 3);
    host_vram_touched();
    return 0;
}

/* mcga_compute_retrace_parameters -- 0d75:0074: how much of the palette fits in a retrace.
   On a fast machine the original's search ends at all 256 colours: count * 14 colours. */
void mcga_compute_retrace_parameters(void)
{
    mcga_palette_fast = 0;
    mcga_retrace_max_colors = 0x100 * 14;
    mcga_retrace_max_bytes = (i16)(mcga_retrace_max_colors * 3);
    mcga_retrace_computed = 1;
}

/* MCGA_SHAKE -- 12ad:000c: one step of a screen shake (the CRTC's start address, which the
   editor never shakes); counts the shake down */
void MCGA_SHAKE(void)
{
    if (MCGA_SHAKES)
        MCGA_SHAKES--;
}
