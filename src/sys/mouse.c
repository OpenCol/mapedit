/* mouse.c -- the mouse: the driver, the software cursor, strokes and the cycle the UI loops run.
 *
 * Strokes. Every UI loop runs mouse_init_cycle once, then mouse_begin_cycle and
 * mouse_end_cycle around each round. mouse_begin_cycle reads the mouse and turns the button
 * state into edges: mouse_start_stroke (a button went down; mouse_button then says which),
 * mouse_stop_stroke (all buttons came up after a press), mouse_changed (anything moved or
 * changed) and mouse_any_stroke (a button is down or has just come up).
 *
 * The cursor. In mode 13h the library draws its own 16 x 16 cursor (byte FFh is transparent)
 * straight onto the screen, saving the pixels under it, and the mouse driver calls its event
 * handler whenever the mouse moves. The editor draws into a work buffer and shows rectangles
 * of it with video_update; mouse_video_update wraps that copy:
 *
 *     mouse_freeze();               moves are only recorded meanwhile
 *     mouse_refresh_view_port();    the cursor drawn into the WORK BUFFER, if it overlaps
 *     video_update(...);            so the copy to the screen carries it
 *     mouse_refresh_done();         and taken out of the work buffer again
 *     mouse_thaw();                 a move made meanwhile is applied
 *
 * This is the original's cursor code in C, step for step, with the INT 33h driver replaced by
 * the host: its mouse events call the event handler (host_set_mouse_handler), already in
 * screen pixels. Only the mode 13h paths exist; the editor runs in nothing else.
 */
#include <string.h>

#include "msc.h"
#include "host.h"
#include "font.h"
#include "loader.h"
#include "menu.h"
#include "mouse.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "screen.h"
#include "sprite.h"
#include "timer.h"
#include "units.h"
#include "video.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */

/* mouse_init_cycle -- starts a UI loop: forgets the old position and takes a button already
 * down as a stroke in progress, so that it starts no new one. */
void mouse_init_cycle(void)
{
    mouse_old_y = mouse_old_x = -1;
    mouse_stroke_going = mouse_status;
    mouse_start_stroke = mouse_latched = 0;
}
/* mouse_begin_cycle -- reads the mouse at the start of a round and sets the stroke flags (see
 * the top of the file); mouse_button is 0 for the left button, 1 for the right. Stamps
 * mouse_clock with timer_read. check runs mouse_check_double first (a no-op in the port). */
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
/* mouse_end_cycle -- ends a round: release calls mouse_double_freedom(-1); wait waits until
 * the clock has moved on from mouse_clock. */
void mouse_end_cycle(i16 release, i16 wait)
{
    if (release)
        mouse_double_freedom(-1);
    if (wait)
        while (timer_read() == mouse_clock)
            timer_idle();
    timer_idle();       /* Port: one round of a UI loop is done: let the machine rest a moment */
}

/* The stroke state mouse_begin_cycle keeps. */
i16 mouse_button = -1;                   /* the last stroke's button: 0 left, 1 right */
i16 mouse_status = 0;                    /* the buttons down: bit 0 left, bit 1 right */
i16 mouse_x = 0;                         /* the mouse position this round */
i16 mouse_y = 0;
i16 mouse_start_stroke = 0;              /* 1: a button went down this round */
i16 mouse_stroke_going = 0;              /* the buttons down last round */
i16 mouse_changed = 0;                   /* 1: moved, or a stroke started or stopped */
i16 mouse_latched = 0;                   /* -1 from a stroke's start until the buttons come up */
i16 mouse_stop_stroke = 0;               /* 1: the buttons came up after a stroke this round */
i16 mouse_any_stroke = 0;                /* 1: a button is down or has just come up */
i16 mouse_old_x = 0;                     /* the position last round (-1: none) */
i16 mouse_old_y = 0;
i32 mouse_clock = 0;                    /* timer_read at the start of the round */

/* mouse_video_init -- makes the whole work screen (scr_work) the cursor code's view port,
 * shown at the screen's top left. */
void mouse_video_init(void)
{
    mouse_set_work_buffer(scr_work.bits, scr_work.w);
    mouse_set_view_port_loc(0, 0, scr_work.w - 1, scr_work.h - 1);
    mouse_set_view_port(0, 0);
}
/* mouse_video_update -- copies the w x h rectangle at (sx, sy) of the work screen to (dx, dy)
 * on the display, with the cursor drawn into it when it overlaps (see the top of the file). */
void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h)
{
    i16 vis;

    mouse_freeze();
    vis = mouse_refresh_view_port();
    video_update(&scr_work, sx, sy, dx, dy, w, h);
    if (vis)
        mouse_refresh_done();
    mouse_thaw();
}

/* The cursor's public state. */
i16 mouse_driver;               /* -1: a mouse is there */
i16 MOUSE_FORCE_BUTTON;         /* buttons pressed through the keypad (keys.c) */
i16 MOUSE_KNOWN_MODE;           /* -1: mode 13h, the library draws the cursor; 0: no cursor */
DosPort MOUSE_CURSOR_BUFFER;    /* the cursor image as a 16 x 16 work buffer */
i8 MOUSE_SHOWING = -1;          /* 0 shown; -1, -2 ...: hidden that often; 80h: thawing */
i16 MOUSE_X_TEMP, MOUSE_Y_TEMP; /* where the cursor is */
i16 MOUSE_VIDEO_MODE;           /* the mode mouse_init was given */

static char arrow[256];         /* the cursor image, 16 x 16 */
/* The screen under the cursor, then the cursor composed over it: one block, because
 * mouse_refresh_view_port reaches the second as the first + 256. */
static char buffers[512];
#define under    (buffers)
#define composed (buffers + 256)
static i16 hot_x, hot_y;        /* the hot spot; never set, so (0, 0) */
static i8 frozen, moved;        /* mouse_freeze: moves recorded, and whether one happened */
static i16 pending_x, pending_y;
static i8 busy;                 /* the handler is running */
static i16 draw_x, draw_y;      /* the drawn cursor's top left on the screen, clipped */
static i16 skip_x, skip_y;      /* how much of the image the clip cut off at the top left */
static u8 rows, cols;           /* how much of it is on the screen */
static i16 image_off;           /* 16 * skip_y + skip_x: where the visible part starts */
static char *screen_at;         /* the screen address of (draw_x, draw_y) */
static i16 double_freedom;      /* stored by mouse_double_freedom, never read */

/* The view port: a work buffer, and which part of it shows where on the screen. */
static char *work;              /* the work buffer's pixels */
static i16 work_w;              /* its width */
static i16 view_x0, view_x1, view_y0, view_y1;   /* the screen rectangle it shows in */
static char *view_origin;       /* the work-buffer pixel at the view's top left */
static u8 saved_rows, saved_cols;               /* what mouse_refresh_done undoes */
static char *saved_under, *saved_work;

/* copy -- copies nrows x ncols bytes, the source rows sstep apart, the destination's dstep. */
static void copy(char *d, const char *s, u8 nrows, u8 ncols, i16 sstep, i16 dstep)
{
    while (nrows--) {
        u8 i;
        for (i = 0; i < ncols; i++)
            d[i] = s[i];
        s += sstep;
        d += dstep;
    }
}

/* overlay -- composes the cursor over what is under it, all 256 pixels. */
static void overlay(void)
{
    u16 i;
    for (i = 0; i < 256; i++)
        composed[i] = (u8)arrow[i] == 0xff ? under[i] : arrow[i];
}

/* draw -- saves the screen under the cursor and draws the cursor there, clipped to the
 * 320 x 200 screen at all four edges. */
static void draw(void)
{
    i16 x = MOUSE_X_TEMP - hot_x, y = MOUSE_Y_TEMP - hot_y, n;
    skip_x = skip_y = 0;
    if (x < 0) { skip_x = (i16)-x; x = 0; }
    draw_x = x;
    if (y < 0) { skip_y = (i16)-y; y = 0; }
    draw_y = y;
    n = y > 0xb8 ? (i16)(0xc8 - y) : 16;
    rows = (u8)(n - skip_y);
    n = x > 0x130 ? (i16)(0x140 - x) : 16;
    cols = (u8)(n - skip_x);
    screen_at = VIDEO_ADDRESS(x, y);
    image_off = (i16)(16 * skip_y + skip_x);
    copy(under + image_off, screen_at, rows, cols, 320, 16);
    overlay();
    copy(screen_at, composed + image_off, rows, cols, 16, 320);
    host_vram_touched();
}

/* wipe -- puts back what the cursor covered. */
static void wipe(void)
{
    copy(screen_at, under + image_off, rows, cols, 16, 320);
    host_vram_touched();
}

/* event -- the driver's event handler, at every move: while frozen it only records the move;
 * a shown cursor is redrawn at (x, y); a hidden one just takes the position (except during
 * mouse_thaw). A call made while the handler runs is dropped. */
static void event(i16 x, i16 y)
{
    if (busy)
        return;
    busy = 1;
    if (frozen) {
        moved = -1;
        pending_x = x;
        pending_y = y;
    } else if (MOUSE_SHOWING == 0) {
        wipe();
        MOUSE_X_TEMP = x;
        MOUSE_Y_TEMP = y;
        draw();
    } else if ((u8)MOUSE_SHOWING != 0x80) {
        MOUSE_X_TEMP = x;
        MOUSE_Y_TEMP = y;
    }
    busy = 0;
}

/* event_move -- the handler's second entry, mouse_thaw's: moves the shown cursor
 * unconditionally. */
static void event_move(i16 x, i16 y)
{
    if (busy)
        return;
    busy = 1;
    wipe();
    MOUSE_X_TEMP = x;
    MOUSE_Y_TEMP = y;
    draw();
    busy = 0;
}

/* host_event -- the host's mouse callback: hands moves to the event handler once mouse_init
 * has started the cursor. The buttons are read by mouse_get_status instead. */
static void host_event(int x, int y, int buttons)
{
    (void)buttons;
    if (mouse_driver && MOUSE_KNOWN_MODE)
        event((i16)x, (i16)y);
}

/* mouse_show -- undoes one mouse_hide; the cursor is drawn when the count reaches 0. */
void mouse_show(void)
{
    if (MOUSE_KNOWN_MODE) {
        if (MOUSE_SHOWING != 0) {
            i8 s = (i8)(MOUSE_SHOWING + 1);
            if (s == 0)
                draw();
            MOUSE_SHOWING = s;
        }
    }
}

/* mouse_hide -- hides the cursor (wiping it if shown); calls nest. */
void mouse_hide(void)
{
    if (MOUSE_KNOWN_MODE) {
        if (MOUSE_SHOWING == 0)
            wipe();
        MOUSE_SHOWING--;
    }
}

/* The cursor image: a 16 x 16 arrow in the original's initialised data, which the editor
 * never changes. Port: this repository carries none of MicroProse's bytes, so the port reads
 * it from the player's own MAPEDIT.EXE in the game directory -- the 1.0 editor, recognised by
 * its size; EXE_CURSOR is where its data segment keeps the image, past the EXE header -- and,
 * without one, draws an arrow of its own. */
#define EXE_SIZE 0x2378cL
#define EXE_CURSOR (0x15e7L * 16 + 0x42d8)

/* cursor_image -- fills arrow, from MAPEDIT.EXE or the port's own drawing. */
static void cursor_image(void)
{
    MSC_FILE *f = msc_fopen("MAPEDIT.EXE", "rb");
    char hdr[0x1c];
    i16 y, x;
    if (f) {
        i32 at;
        int ok = msc_fread(hdr, 1, sizeof hdr, f) == sizeof hdr && hdr[0] == 'M' && hdr[1] == 'Z';
        if (ok) {
            at = (i32)((u8)hdr[8] | (u8)hdr[9] << 8) * 16 + EXE_CURSOR;
            ok = msc_filelength(f->_file) == EXE_SIZE && msc_fseek(f, at, SEEK_SET) == 0
                 && msc_fread(arrow, 1, sizeof arrow, f) == sizeof arrow;
        }
        msc_fclose(f);
        if (ok)
            return;
    }
    /* the port's own: black outline, white inside, FFh around it */
    memset(arrow, 0xff, sizeof arrow);
    for (y = 0; y < 13; y++)
        for (x = 0; x <= y && x < 10; x++) {
            int edge = x == 0 || x == y || y == 12 || (y >= 10 && x >= 9 - (y - 10));
            arrow[y * 16 + x] = (char)(edge ? 0 : 15);
        }
}

/* mouse_init -- sets the video mode and starts the mouse for it: the cursor hidden, centred
 * on (160, 100), drawn by the library in mode 13h. on = 0 switches the mouse off instead (the
 * shutdown does that with text mode 3). Returns mouse_driver: -1 when on. Port: there is
 * always a mouse. */
i16 mouse_init(i16 on, i16 mode)
{
    static int have_image;
    if (!have_image) {
        cursor_image();
        have_image = 1;
    }
    video_init(mode, 0);
    MOUSE_CURSOR_BUFFER.h = 16;
    MOUSE_CURSOR_BUFFER.w = 16;
    MOUSE_CURSOR_BUFFER.bits = arrow;
    MOUSE_VIDEO_MODE = mode;
    mouse_driver = 0;
    MOUSE_KNOWN_MODE = 0;
    if (!on)
        return mouse_driver;
    mouse_driver = -1;
    MOUSE_SHOWING = -1;
    frozen = 0;
    MOUSE_X_TEMP = 160;
    MOUSE_Y_TEMP = 100;
    MOUSE_FORCE_BUTTON = 0;
    moved = 0;
    if (mode == 0x13)
        MOUSE_KNOWN_MODE = -1;
    host_set_mouse_handler(host_event);
    host_mouse_warp(MOUSE_X_TEMP, MOUSE_Y_TEMP);
    return mouse_driver;
}

/* The original's "double" entry points have nothing to do in the port: mouse_double_freedom
 * stores its argument, mouse_check_double is empty. */
void mouse_double_freedom(i16 n) { double_freedom = n; }
void mouse_check_double(void) { }

/* mouse_freeze -- holds the cursor still: moves are recorded until mouse_thaw. */
void mouse_freeze(void)
{
    if (MOUSE_KNOWN_MODE) {
        moved = 0;
        frozen = -1;
    }
}

/* mouse_thaw -- lets the cursor move again, applying the last move recorded while frozen. */
void mouse_thaw(void)
{
    if (MOUSE_KNOWN_MODE) {
        i8 was = MOUSE_SHOWING;
        MOUSE_SHOWING = (i8)0x80;
        frozen = 0;
        if (moved && was == 0)
            event_move(pending_x, pending_y);
        MOUSE_SHOWING = was;
    }
}

/* mouse_get_status -- stores the cursor's position in *x, *y and returns the buttons down (bit
 * 0 left, bit 1 right): the host's, and any the keypad holds down. */
i16 mouse_get_status(i16 *x, i16 *y)
{
    int b = 0;
    if (mouse_driver)
        host_mouse(NULL, NULL, &b);     /* also runs the event loop: the handler may move it */
    *x = MOUSE_X_TEMP;
    *y = MOUSE_Y_TEMP;
    return (i16)(b | MOUSE_FORCE_BUTTON);
}

/* mouse_set_work_buffer -- the work buffer the view port shows, and its width. */
void mouse_set_work_buffer(char *bits, i16 w)
{
    work = bits;
    work_w = w;
}

/* mouse_set_view_port_loc -- the part of the screen the view port shows (inclusive). */
void mouse_set_view_port_loc(i16 x0, i16 y0, i16 x1, i16 y1)
{
    view_x0 = x0;
    view_y0 = y0;
    view_x1 = x1;
    view_y1 = y1;
}

/* mouse_set_view_port -- the work-buffer pixel shown at the view port's top left. */
void mouse_set_view_port(i16 x, i16 y)
{
    view_origin = work + (u16)((u16)y * (u16)work_w + (u16)x);
}

/* mouse_refresh_view_port -- if the shown cursor overlaps the view port, draws it into the
 * work buffer over the new picture, so that the next video_update shows it, and takes the new
 * picture as what is under the cursor. Returns -1 when it did, 0 when not. Quirk of the
 * original: a cursor cut by the view's top or left edge is not also cut at its bottom or
 * right, which matters only for a view shorter or narrower than the cursor. */
i16 mouse_refresh_view_port(void)
{
    i16 vis_w, vis_h, cx, dx, ax, bx;
    i16 row0, col0;             /* where in the view (work-buffer rows, columns) */
    i16 img_row, img_col;       /* where in the visible part of the image */
    u8 nrows, ncols;
    if (!MOUSE_KNOWN_MODE || MOUSE_SHOWING != 0)
        return 0;
    vis_w = (i16)(16 - skip_x);
    vis_h = (i16)(16 - skip_y);
    cx = draw_x;
    if (cx > view_x1)
        return 0;
    dx = (i16)(cx + vis_w - 1);
    if (dx < view_x0)
        return 0;
    ax = draw_y;
    if (ax > view_y1)
        return 0;
    bx = (i16)(ax + vis_h - 1);
    if (bx < view_y0)
        return 0;
    /* rows: clipped against the view's top and bottom */
    if (ax - view_y0 < 0) {
        img_row = (i16)(view_y0 - ax);
        nrows = (u8)(bx - view_y0 + 1);
        row0 = 0;
    } else {
        row0 = (i16)(ax - view_y0);
        img_row = 0;
        nrows = (u8)(bx - view_y1 > 0 ? vis_h - (bx - view_y1) : vis_h);
    }
    /* columns: the same against its left and right */
    if (cx - view_x0 < 0) {
        img_col = (i16)(view_x0 - cx);
        ncols = (u8)(dx - view_x0 + 1);
        col0 = 0;
    } else {
        col0 = (i16)(cx - view_x0);
        img_col = 0;
        ncols = (u8)(dx - view_x1 > 0 ? vis_w - (dx - view_x1) : vis_w);
    }
    saved_work = view_origin + (u16)((u16)row0 * (u16)work_w + (u16)col0);
    saved_under = under + image_off + 16 * img_row + img_col;
    saved_rows = nrows;
    saved_cols = ncols;
    copy(saved_under, saved_work, nrows, ncols, work_w, 16);      /* the new picture under it */
    overlay();
    copy(saved_work, saved_under + 256, nrows, ncols, 16, work_w); /* composed = under + 256 */
    return -1;
}

/* mouse_refresh_done -- takes the cursor out of the work buffer again. */
void mouse_refresh_done(void)
{
    if (saved_work)
        copy(saved_work, saved_under, saved_rows, saved_cols, 16, work_w);
}

/* mouse_keypad_move -- keys_tick's keypad step (keys.c) moves the cursor through the event
 * handler. */
void mouse_keypad_move(i16 x, i16 y)
{
    event(x, y);
}
