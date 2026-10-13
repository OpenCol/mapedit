/* Imported from mapedit-decomp src/madsdev/mouse_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
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
 * MOUSE STATE (root code, frame 0acb). The driver read is mouse_read (0a58:038b: buttons, &x, &y).
 *   - mouse_in_box(x0, y0, x1, y1) (_fastcall) tests the mouse against a box.
 *   - input_reset forgets the previous position and re-reads the buttons. It stamps the tick.
 *   - sync_frame(a, hold) (_fastcall) waits for the next tick when `hold`.
 *
 *  Three things got it there, in this order:
 *   - the button word is NOT a named local in the shipped source. Dropping `int b` and
 *     reading g_mouse_buttons at each of its six uses gives the size-0 bp frame the shipped
 *     code has (`push bp; mov bp,sp`, `mov sp,bp` after the driver read, `leave`) instead of
 *     our `push si` / `add sp,4` / `pop si`. A CSE needs no
 *     callee-saved register, so the frame comes back. That took 45 bytes to 17.
 *   - the in-bar test reads the clicked flag as `(&g_mouse_clicked)[0]`: as a plain scalar
 *     MSC reuses the value it just stored.
 *   - the last 17 bytes were one register -- the CSE of g_mouse_buttons sat in dx where
 *     shipped has it in bx (hence `mov al,bl`) -- and a VOLATILE STORE is what moves it:
 *     `(*(volatile int *)&g_mouse_pressed) = pressed;`. The store MSC may not drop or fold
 *     changes what it has to keep live across it, and the CSE lands in bx.
 *     Nothing else had moved it: pads 0..16, spare locals before or after `pressed`,
 *     `register int pressed`, four spellings of the pressed expression, a volatile READ of
 *     the global, pragmas e/l/g/a/c off in any combination, declaration order, cdecl/pascal,
 *     and extra stack parameters (they bring retf N, still no frame).
 *   - input_reset's first statement had to become `g_mouse_prev_y = g_mouse_prev_x = -1;`.
 *     In a chained assignment MSC stores the RIGHTMOST target first, so the old
 *     `prev_x = prev_y = -1` put prev_y at 0x7f8. The other two chains in the same function were already the right way round
 *     (`g_dragging = g_mouse_buttons = ...` stores 0x7e6 then 0x7ee, as shipped does).
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void mouse_check_double(void);
extern i16 mouse_get_status(i16 *x, i16 *y);
extern void mouse_double_freedom(i16 n);
extern i32 timer_read(void);
extern i16 mouse_button;                 /* DS:0x0720 */
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
    i16 b;
    i16 start;

    if (check)
        mouse_check_double();
    mouse_old_x = mouse_x;
    mouse_old_y = mouse_y;
    mouse_status = mouse_get_status(&mouse_x, &mouse_y);
    mouse_clock = timer_read();
    b = mouse_status;
    if (mouse_latched != 0 && b == 0)
        mouse_stop_stroke = 1;
    else
        mouse_stop_stroke = 0;
    if (b != 0 && mouse_stroke_going == 0)
        start = 1;
    else
        start = 0;
    mouse_stroke_going = b;
    if (b == 0)
        mouse_latched = 0;
    if (mouse_old_x == mouse_x && mouse_old_y == mouse_y && start == 0 && mouse_stop_stroke == 0)
        mouse_changed = 0;
    else
        mouse_changed = 1;
    mouse_start_stroke = start;
    if (start) {
        mouse_latched = -1;
        mouse_button = (b & 1) == 0;
    }
    if (b == 0 && mouse_stop_stroke == 0)
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

