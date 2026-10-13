/* From mapedit-decomp src/madsdev/mouse_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE mouse_2: MAPEDIT.EXE segment 0c21 (mouse_2.c), file 0xd810-0xd94c, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mouse_in_box      0x0d810
 * fn mouse_init_cycle  0x0d83a
 * fn mouse_begin_cycle 0x0d852
 * fn mouse_end_cycle   0x0d920
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0acb).
 * The other three have no VICEROY twin: reconstructed from the shipped bytes. Their globals
 * are the CodeView ones (mouse_x at DS:0x0724 ...); the VICEROY draft's g_mouse_* names and
 * DS:0x07e4 addresses below were the VICEROY layout and are replaced.
 *
 * MOUSE STATE (madsdev mouse_2.c; "elg"). The driver is mouse_1.ASM (mouse_get_status,
 * mouse_check_double, mouse_double_freedom).
 *   - mouse_in_box(x0, y0, x1, y1) (_fastcall) tests the mouse against a box.
 *   - mouse_init_cycle forgets the old position, takes the current button state as the
 *     stroke in progress and clears the latch.
 *   - mouse_begin_cycle(check) (_fastcall) reads the mouse and derives: mouse_stop_stroke
 *     (a release after a latched press), mouse_start_stroke (a new press, mouse_button then
 *     telling the right button from the left), mouse_changed (moved or any stroke edge) and
 *     mouse_any_stroke. It stamps mouse_clock.
 *   - mouse_end_cycle(release, wait) (_fastcall) releases the double-click guard and waits
 *     for the next timer tick when `wait`.
 */


extern void mouse_check_double(void);
extern i16 mouse_get_status(i16 *x, i16 *y);
extern void mouse_double_freedom(i16 n);
extern i32 timer_read(void);
extern i16 mouse_button;                   /* DS:0x0720  */
extern i16 mouse_status;                 /* DS:0x0722 */
extern i16 mouse_x;                      /* DS:0x0724 */
extern i16 mouse_y;                      /* DS:0x0726 */
extern i16 mouse_start_stroke;           /* DS:0x0728 */
extern i16 mouse_stroke_going;           /* DS:0x072a */
extern i16 mouse_changed;                /* DS:0x072c */
extern i16 mouse_latched;                /* DS:0x072e */
extern i16 mouse_stop_stroke;            /* DS:0x0730 */
extern i16 mouse_any_stroke;             /* DS:0x0732 */
extern i16 mouse_old_x;                  /* DS:0x0734 */
extern i16 mouse_old_y;                  /* DS:0x0736 */
extern i32 mouse_clock;                 /* DS:0x0738 */
/* mouse_in_box -- 0c21:0000, 42 bytes */
i16 mouse_in_box(i16 x0, i16 y0, i16 x1, i16 y1)
{
    if (x0 <= mouse_x && x1 >= mouse_x && y0 <= mouse_y && y1 >= mouse_y)
        return 1;
    return 0;
}
/* mouse_init_cycle -- 0c21:002a, 24 bytes */
void mouse_init_cycle(void)
{
    mouse_old_y = mouse_old_x = -1;
    mouse_stroke_going = mouse_status;
    mouse_start_stroke = mouse_latched = 0;
}
/* mouse_begin_cycle -- 0c21:0042, 206 bytes */
void mouse_begin_cycle(i16 check)
{
    i16 start;

    if (check)
        mouse_check_double();
    mouse_old_x = mouse_x;
    mouse_old_y = mouse_y;
    mouse_status = mouse_get_status(&mouse_x, &mouse_y);
    mouse_clock = timer_read();
    if (mouse_latched != 0 && mouse_status == 0)
        *(volatile i16 *)&mouse_stop_stroke = 1;
    else
        *(volatile i16 *)&mouse_stop_stroke = 0;
    if (mouse_status != 0 && mouse_stroke_going == 0)
        start = 1;
    else
        start = 0;
    mouse_stroke_going = mouse_status;
    if (mouse_status == 0)
        mouse_latched = 0;
    if (mouse_old_x == mouse_x && mouse_old_y == mouse_y && start == 0 && mouse_stop_stroke == 0)
        mouse_changed = 0;
    else
        mouse_changed = 1;
    mouse_start_stroke = start;
    if (start) {
        mouse_latched = -1;
        mouse_button = (mouse_status & 1) == 0;
    }
    if (mouse_status == 0 && mouse_stop_stroke == 0)
        mouse_any_stroke = 0;
    else
        mouse_any_stroke = 1;
}
/* mouse_end_cycle -- 0c21:0110, 44 bytes */
void mouse_end_cycle(i16 release, i16 wait)
{
    if (release)
        mouse_double_freedom(-1);
    if (wait)
        while (timer_read() == mouse_clock)
            ;
}

/* The mouse state, defined here after the code (docs/msc6-codegen.md: a definition after the last
 * function moves no symbol position; defined before mouse_begin_cycle it swaps that
 * function's bx/dx). The 13 land at DS:0x0720-0x073c in this order; the module has no
 * literals.
 */
i16 mouse_button = -1;                   /* DS:0x0720 */
i16 mouse_status = 0;                    /* DS:0x0722 */
i16 mouse_x = 0;                         /* DS:0x0724 */
i16 mouse_y = 0;                         /* DS:0x0726 */
i16 mouse_start_stroke = 0;              /* DS:0x0728 */
i16 mouse_stroke_going = 0;              /* DS:0x072a */
i16 mouse_changed = 0;                   /* DS:0x072c */
i16 mouse_latched = 0;                   /* DS:0x072e */
i16 mouse_stop_stroke = 0;               /* DS:0x0730 */
i16 mouse_any_stroke = 0;                /* DS:0x0732 */
i16 mouse_old_x = 0;                     /* DS:0x0734 */
i16 mouse_old_y = 0;                     /* DS:0x0736 */
i32 mouse_clock = 0;                    /* DS:0x0738 */

