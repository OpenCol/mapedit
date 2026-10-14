/* mouse.h -- the mouse: the driver, the software cursor, strokes.
 *
 * Defined in sys/mouse.c. mouse_init starts the mouse with the cursor hidden; mouse_show and
 * mouse_hide nest. The cursor is a 16 x 16 image the library draws itself onto the screen,
 * saving what it covers; mouse_freeze and mouse_thaw hold it still while the program writes
 * the screen, and mouse_video_update copies a rectangle of the work screen to the display
 * with the cursor drawn into it.
 *
 * The UI loops read the mouse once a round: mouse_begin_cycle takes the position and the
 * buttons and derives the stroke state below (a stroke runs from a button going down to all
 * buttons coming up), and mouse_end_cycle closes the round. mouse_keypad_move is how
 * keys_tick (keys.h) moves the cursor for the keypad. The double-click entry points do nothing in the port.
 */
#ifndef MAPEDIT_MOUSE_H
#define MAPEDIT_MOUSE_H

#include "msc.h"

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
void mouse_init_cycle(void);
void mouse_video_init(void);
void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

extern DosPort MOUSE_CURSOR_BUFFER;  /* the cursor image as a 16 x 16 port */
extern i16 MOUSE_FORCE_BUTTON;       /* buttons the keypad holds down */
extern i16 MOUSE_KNOWN_MODE;         /* -1: mode 13h, the library draws the cursor */
extern i8 MOUSE_SHOWING;             /* 0 shown; -1, -2 ...: hidden that often */
extern i16 MOUSE_VIDEO_MODE;         /* the mode mouse_init was given */
extern i16 MOUSE_X_TEMP;             /* where the cursor is */
extern i16 MOUSE_Y_TEMP;
extern i16 mouse_driver;             /* -1: there is a mouse (always, in the port) */
/* the stroke state, from mouse_begin_cycle */
extern i16 mouse_any_stroke;         /* a button is down or has just come up */
extern i16 mouse_button;             /* the last stroke's button: 0 left, 1 right */
extern i16 mouse_changed;            /* moved, or a stroke started or stopped */
extern i32 mouse_clock;              /* timer_read at the start of the round */
extern i16 mouse_latched;            /* -1 from a stroke's start until its end */
extern i16 mouse_old_x;              /* the position last round (-1: none) */
extern i16 mouse_old_y;
extern i16 mouse_start_stroke;       /* a button went down this round */
extern i16 mouse_status;             /* the buttons down: bit 0 left, bit 1 right */
extern i16 mouse_stop_stroke;        /* the stroke ended this round */
extern i16 mouse_stroke_going;       /* the buttons down last round */
extern i16 mouse_x;                  /* the position this round */
extern i16 mouse_y;

#endif
