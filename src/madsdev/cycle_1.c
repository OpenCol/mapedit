/* From mapedit-decomp src/madsdev/cycle_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE cycle_1: MAPEDIT.EXE segment 0f1a (cycle_1.c), file 0x107aa-0x1096a, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn cycle_init   0x107aa
 * fn cycle_colors 0x10846
 *
 * Near twin: `root_0a0a` cycle_start / cycle_tick (VICEROY's copy keeps the
 * palette far and reads the ranges from a loaded file; here the ranges are copied in and
 * the cycling palette is a copy of master_palette).
 *
 * PALETTE COLOUR CYCLING (madsdev cycle_1.c). cycle_list holds up to eight ranges
 * {count, pos, start, speed}.
 *   - cycle_init(ranges, on) copies the 34-byte range list and the 768-byte master palette
 *     in, stamps every range with the current tick, totals the cycled colours
 *     (total_cycle_colors), sets cycling_threshold (3, or 0 for 16 colours or fewer) and
 *     turns cycling on or off.
 *   - cycle_colors, called per tick, rotates each due range of the cycling palette one
 *     entry (inline assembly, through a 3-byte temp) and uploads the cycled part with
 *     mcga_setpal_range once cycling_delay reaches the threshold.
 */

typedef struct { u8 count, pos, start, speed; } CycleRange;
typedef struct { i16 n; CycleRange r[8]; } ColorCycle;

extern i32 timer_read(void);
ColorCycle cycle_list;                   /* DS:0x5afe (BSS) */
i32 cycle_timing[8];                    /* DS:0x52e8 (BSS) */
i16 total_cycle_colors;                  /* DS:0x4e9a (BSS) */
i16 cycling_threshold;                   /* DS:0x4b10 (BSS) */
i16 cycling_active = 0;                  /* DS:0x3c64 (defined here) */
extern u8 master_palette[0x300];   /* DS:0x5b20 */
u8 cycling_palette[0x300];    /* DS:0x6048 (BSS) */
/* cycle_init -- 0f1a:000a, 156 bytes */
void cycle_init(ColorCycle *ranges, i16 on)
{
    i32 now;
    i16 i;

    now = timer_read();
    for (i = 0; i < 8; i++)
        cycle_timing[i] = now;
    msc__fmemcpy(&cycle_list, ranges, 0x22);
    msc__fmemcpy(cycling_palette, master_palette, 0x300);
    total_cycle_colors = 0;
    for (i = 0; i < cycle_list.n; i++) {
        total_cycle_colors += cycle_list.r[i].count;
        cycle_list.r[i].pos = 0;
    }
    cycling_threshold = 3;
    if (total_cycle_colors <= 16)
        cycling_threshold = 0;
    cycling_active = on;
}
/* cycle_colors -- 0f1a:00a6, 292 bytes */
i16 cycling_delay = 0;                   /* DS:0x3c66 (defined here) */
extern i16 mcga_palette_update;          /* DS:0x3ab6 */
static u8 cycle_rgb[3];       /* DS:0x49a6: module static (no public) */
extern void mcga_setpal_range(i16 first, i16 count, char *pal);

/* PORT: cycle_colors is in port/ (inline assembly) */
void cycle_colors(void);


