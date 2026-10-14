/* mcga.h -- the VGA palette: writing and reading the DAC in step with the vertical retrace.
 *
 * Defined in ui/palette.c. A palette is 256 RGB triples of 6-bit values (768 bytes). The
 * original wrote the DAC a chunk at a time, each chunk after the start of a vertical retrace
 * so that colour changes never tear, and measured at startup how much fits in one retrace
 * (mcga_compute_retrace_parameters). The port's host shows the DAC at its next 70 Hz frame
 * and reports a fast machine: every colour in one retrace.
 */
#ifndef MAPEDIT_MCGA_H
#define MAPEDIT_MCGA_H

#include "msc.h"

void mcga_compute_retrace_parameters(void);
void mcga_getpal(char *pal);
void mcga_retrace(void);
void mcga_setpal(char *pal);
void mcga_setpal_range(i16 first, i16 count, char *pal);
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal);

extern i16 mcga_palette_fast;          /* the original's fast DAC write; not read */
extern i16 mcga_palette_update;        /* nonzero while a DAC transfer runs */
extern i16 mcga_retrace_computed;      /* the retrace has been measured; not read */
extern i16 mcga_retrace_max_bytes;     /* bytes written per retrace: 3 a colour */
extern i16 mcga_retrace_max_colors;    /* colours written per retrace */
extern u16 mcga_retrace_ticks;         /* a frame in PIT ticks; unused */

#endif
