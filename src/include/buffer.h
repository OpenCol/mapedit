/* buffer.h -- the functions, data and records of buffer.c, buffer_1.c, buffer_2.c, buffer_3.c, buffer_7.c, buffer_8.c, buffer_b.c, buffer_i.c, buffer_r.c, buffer_t.c, buffer_z.c. */
#ifndef MAPEDIT_BUFFER_H
#define MAPEDIT_BUFFER_H

#include "msc.h"

void buffer_hline(i16 x0, i16 x1, i16 y, DosPort port, char color);
char *buffer_pointer(DosPort *port, i16 x, i16 y);
i16 buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h);
i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h);
i16 buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);
void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color);
i16 buffer_init(DosPort *port, u16 w, u16 h);
i16 buffer_init_name(DosPort *port, u16 w, u16 h, char *tag);
i16 buffer_free(DosPort *port);
void buffer_fill(char c, DosPort port);
void buffer_put(i16 x, i16 y, i16 v, DosPort port);
u8 buffer_get(i16 x, i16 y, DosPort port);
void buffer_draw_box(i16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color);
i16 buffer_conform(DosPort *port, i16 *x, i16 *y, i16 *w, i16 *h);
void buffer_from_disk(DosPort *port, i16 slot, i16 keep, i16 x, i16 y, i16 w, i16 h);
i16 buffer_to_disk(DosPort *port, i16 x, i16 y, i16 w, i16 h);
i16 buffer_preserve(DosPort *port, i16 mode, i16 src, i16 x, i16 y, i16 w, i16 h);
void buffer_restore(DosPort *port, i16 handle, i16 dst, i16 x, i16 y, i16 w, i16 h);
void buffer_tile(DosPort dst, DosPort src, i16 x, i16 y, i16 w, i16 h, i16 sx, i16 sy);

extern u8 buffer_accum;
extern char buffer_disk_filename[];
extern u8 buffer_tracking[10];
extern i16 buffer_restore_keep_flag;

#endif
