/* From mapedit-decomp src/madsdev/mcga_c (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE mcga_c: MAPEDIT.EXE segment 0d75 (mcga_c.c), file 0xed5e-0xeef0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_time_palette_swap          0x0ed5e
 * fn mcga_compute_retrace_parameters 0x0edc4
 *
 * Near twins: `ovl30_77ed0` dac_write_range / dac_calibrate (VICEROY's later
 * copy of this module).
 *
 * TIMING THE PALETTE WRITE (madsdev mcga_c.c).
 *   - mcga_time_palette_swap(first, count, pal) (_fastcall) writes `count` DAC entries from
 *     `first` with interrupts off, timed against PIT channel 0, and returns the elapsed
 *     ticks. With mcga_palette_fast set it uses one `rep outsb`; otherwise a plain
 *     outsb/loop (VICEROY adds two I/O-delay jumps per byte).
 *   - mcga_compute_retrace_parameters measures how many palette entries fit in one frame:
 *       1. it times 32 vertical retraces against PIT channel 0 (inline asm) and keeps the
 *          average as mcga_retrace_ticks;
 *       2. it reads the palette and binary-searches the largest count the timed write
 *          fits within a frame (the step starts at 0x40 and halves, 0x20 tries at most;
 *          VICEROY allows 0x40);
 *       3. it records count * 14 (0x20 when nothing fitted) as mcga_retrace_max_colors,
 *          three times that as mcga_retrace_max_bytes, and sets mcga_retrace_computed.
 *     Its frame is 0x60a bytes: a second, unused 768-byte buffer (VICEROY's copy has only
 *     the one, frame 0x30a).
 */

extern i16 mcga_retrace_computed;           /* DS:0x3aae */
extern u16 mcga_retrace_ticks;         /* DS:0x3ab0 */
extern i16 mcga_retrace_max_colors;         /* DS:0x3ab2 */
extern i16 mcga_retrace_max_bytes;          /* DS:0x3ab4 */
extern i16 mcga_palette_fast;               /* DS:0x3ab8 */
extern void mcga_getpal(char *pal);
/* mcga_time_palette_swap -- 0d75:000e, 102 bytes */
/* PORT: mcga_time_palette_swap is in port/ (inline assembly) */
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal);

/* mcga_compute_retrace_parameters -- 0d75:0074, 300 bytes */
/* 16 pads: the `spent > mcga_retrace_ticks` compare loads the global into ax (position mod 17);
 * at 0..13 pads ours is `cmp [ticks],ax`. */
/* PORT: mcga_compute_retrace_parameters is in port/ (inline assembly) */
void mcga_compute_retrace_parameters(void);


