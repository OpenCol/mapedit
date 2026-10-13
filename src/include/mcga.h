/* mcga.h -- the functions, data and records of mcga.c, mcga_7.c, mcga_d.c. */
#ifndef MAPEDIT_MCGA_H
#define MAPEDIT_MCGA_H

#include "msc.h"

void mcga_compute_retrace_parameters(void);
void mcga_getpal(char *pal);
void mcga_retrace(void);
void mcga_setpal(char *pal);
void mcga_setpal_range(i16 first, i16 count, char *pal);
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal);

extern i16 mcga_palette_fast;
extern i16 mcga_palette_update;
extern i16 mcga_retrace_computed;
extern i16 mcga_retrace_max_bytes;
extern i16 mcga_retrace_max_colors;
extern u16 mcga_retrace_ticks;

#endif
