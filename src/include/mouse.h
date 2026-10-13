/* mouse.h -- the functions, data and records of mouse.c, mouse_2.c, mouse_4.c. */
#ifndef MAPEDIT_MOUSE_H
#define MAPEDIT_MOUSE_H

#include "msc.h"

void MOUSE_BEGIN_DOUBLE(i16 a, i16 b, i16 c, i16 d);
void MOUSE_CHANGE_CURSOR_BEGIN(void);
void MOUSE_CHANGE_CURSOR_END(void);
void MOUSE_DISABLE_SCALE(void);
void MOUSE_END_DOUBLE(void);
void MOUSE_FORCE(i16 x, i16 y);
i16 MOUSE_GET_VIDEO_MODE(void);
void MOUSE_HARD_CURSOR_MODE(i16 mode, char *colors);
void MOUSE_HORIZ_BOUND(i16 lo, i16 hi);
void MOUSE_SCREEN_SWAP(i16 mode);
void MOUSE_SET_HOTSPOT(i16 x, i16 y);
void MOUSE_TIMING(void);
void MOUSE_VERT_BOUND(i16 lo, i16 hi);
void mouse_check_double(void);
void mouse_double_freedom(i16 n);
void mouse_freeze(void);
i16 mouse_get_status(i16 *x, i16 *y);
void mouse_hide(void);
i16 mouse_init(i16 on, i16 mode);
void mouse_keypad_move(i16 x, i16 y);
void mouse_refresh_done(void);
i16 mouse_refresh_view_port(void);
void mouse_set_view_port(i16 x, i16 y);
void mouse_set_view_port_loc(i16 x0, i16 y0, i16 x1, i16 y1);
void mouse_set_work_buffer(char *bits, i16 w);
void mouse_show(void);
void mouse_thaw(void);
void mouse_begin_cycle(i16 check);
void mouse_end_cycle(i16 release, i16 wait);
i16 mouse_in_box(i16 x0, i16 y0, i16 x1, i16 y1);
void mouse_init_cycle(void);
void mouse_video_init(void);
void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

extern DosPort MOUSE_CURSOR_BUFFER;
extern i16 MOUSE_FORCE_BUTTON;
extern i16 MOUSE_KNOWN_MODE;
extern i8 MOUSE_SHOWING;
extern i16 MOUSE_VIDEO_MODE;
extern i16 MOUSE_X_TEMP;
extern i16 MOUSE_Y_TEMP;
extern i16 mouse_driver;
extern i16 mouse_any_stroke;
extern i16 mouse_button;
extern i16 mouse_changed;
extern i32 mouse_clock;
extern i16 mouse_latched;
extern i16 mouse_old_x;
extern i16 mouse_old_y;
extern i16 mouse_start_stroke;
extern i16 mouse_status;
extern i16 mouse_stop_stroke;
extern i16 mouse_stroke_going;
extern i16 mouse_x;
extern i16 mouse_y;

#endif
