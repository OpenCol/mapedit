/* mouse.c -- mouse_1.ASM: MicroProse's mouse cursor, mode 13h.
 *
 * In mode 13h the library draws its own 16 x 16 cursor (byte FFh is transparent) straight
 * onto the screen, saving the pixels under it, and asks the mouse driver to call its event
 * handler (MOUSE_CURSOR) whenever the mouse moves. The editor draws into a work buffer and
 * shows rectangles of it with video_update; around that it calls
 *
 *     mouse_freeze();               moves are only recorded meanwhile
 *     mouse_refresh_view_port();    the cursor drawn into the WORK BUFFER, if it overlaps
 *     video_update(...);            so the copy to the screen carries it
 *     mouse_refresh_done();         and taken out of the work buffer again
 *     mouse_thaw();                 a move made meanwhile is applied
 *
 * This is that code in C, step for step, with the INT 33h driver replaced by the host: its
 * mouse events call the event handler (host_set_mouse_handler), already in screen pixels,
 * so the driver's halving of x (640-wide virtual coordinates) is gone. Only the mode 13h
 * paths exist; the editor runs in nothing else. Offsets are mouse_1.ASM's.
 */
#include <string.h>

#include "msc.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "sprite.h"
#include "mouse.h"
#include "records.h"
#include "units.h"
#include "video.h"
#include "host.h"


/* the public state, as named in MAPEDIT.EXE's symbol table */
i16 mouse_driver;               /* -1: a mouse is there */
i16 MOUSE_FORCE_BUTTON;         /* buttons pressed through the keypad (keys.c) */
i16 MOUSE_KNOWN_MODE;           /* -1: mode 13h, the library draws the cursor */
DosPort MOUSE_CURSOR_BUFFER;    /* the cursor image as a 16 x 16 work buffer */
i8 MOUSE_SHOWING = -1;          /* 0: shown; -1, -2 ...: hidden that many times */
i16 MOUSE_X_TEMP, MOUSE_Y_TEMP; /* where the cursor is */
i16 MOUSE_VIDEO_MODE;

static char arrow[256];         /* DS:42d8, the image */
/* DS:3cd0 the screen under the cursor, then DS:3dd0 the cursor composed over it: one block,
   because mouse_refresh_view_port reaches the second as the first + 100h */
static char buffers[512];
#define under    (buffers)
#define composed (buffers + 256)
static i16 hot_x, hot_y;        /* the hot spot */
static i8 frozen, moved;        /* mouse_freeze: moves recorded, and whether one happened */
static i16 pending_x, pending_y;
static i8 busy;                 /* DS:42d6: the handler is running */
static i16 draw_x, draw_y;      /* the drawn cursor's top left on the screen, clipped */
static i16 skip_x, skip_y;      /* how much of the image the clip cut off at the top left */
static u8 rows, cols;           /* how much of it is on the screen */
static i16 image_off;           /* 16 * skip_y + skip_x: where the visible part starts */
static char *screen_at;         /* the screen address of (draw_x, draw_y) */
static i16 double_freedom;

/* the view port: a work buffer, and which part of it shows where on the screen */
static char *work;              /* DS:3cb2 */
static i16 work_w;              /* DS:3cb4 */
static i16 view_x0, view_x1, view_y0, view_y1;   /* DS:3cb6, 3cb8, 3cba, 3cbc: on the screen */
static char *view_origin;       /* DS:3cc4: the work-buffer pixel at the view's top left */
static u8 saved_rows, saved_cols;               /* DS:3cca .. 3cce: what refresh_done undoes */
static char *saved_under, *saved_work;

/* MOUSE_COPY -- 0731: rows x cols bytes, the source rows sstep apart, the destination dstep */
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

/* MOUSE_OVERLAY_VGA -- 0746: the cursor over what is under it, all 256 pixels */
static void overlay(void)
{
    u16 i;
    for (i = 0; i < 256; i++)
        composed[i] = (u8)arrow[i] == 0xff ? under[i] : arrow[i];
}

/* MOUSE_DRAW -- 066f: save the screen under the cursor and draw it there */
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

/* MOUSE_WIPE -- 064a: put back what the cursor covered */
static void wipe(void)
{
    copy(screen_at, under + image_off, rows, cols, 16, 320);
    host_vram_touched();
}

/* MOUSE_CURSOR -- 03f8: the driver's event handler, at every move */
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

/* 045d: the handler's second entry, mouse_thaw's: move the shown cursor unconditionally */
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

static void host_event(int x, int y, int buttons)
{
    (void)buttons;
    if (mouse_driver && MOUSE_KNOWN_MODE)
        event((i16)x, (i16)y);
}

/* mouse_show -- 0007 */
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

/* mouse_hide -- 004e */
void mouse_hide(void)
{
    if (MOUSE_KNOWN_MODE) {
        if (MOUSE_SHOWING == 0)
            wipe();
        MOUSE_SHOWING--;
    }
}

/* The cursor image: mouse_1.ASM's initialised data (DS:42D8, a 16 x 16 arrow), which the
   editor never changes. This repository carries none of MicroProse's bytes, so the port
   reads it from the player's own MAPEDIT.EXE in the game directory -- the 1.0 editor, whose
   data segment is paragraph 15E7h -- and, without one, draws an arrow of its own. */
#define EXE_SIZE 0x2378cL
#define EXE_CURSOR (0x15e7L * 16 + 0x42d8)

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

/* mouse_init -- 0086: start the mouse for a video mode (on = 0 turns it off). Returns
   mouse_driver. The port always has a mouse. */
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

/* MOUSE_SET_HOTSPOT -- 01f4 */
void MOUSE_SET_HOTSPOT(i16 x, i16 y)
{
    hot_x = (i16)(x & 15);
    hot_y = (i16)(y & 15);
}

/* MOUSE_TIMING -- 020d: wait for the BIOS clock to tick (18.2 a second) */
void MOUSE_TIMING(void)
{
    u32 t = host_ms() * 182u / 10000u;
    while (host_ms() * 182u / 10000u == t)
        host_sleep(1);
}

i16 MOUSE_GET_VIDEO_MODE(void) { return MOUSE_VIDEO_MODE; }

/* the double-screen cursor (for a second monitor) has no use in the port; these keep its
   bookkeeping only */
void mouse_double_freedom(i16 n) { double_freedom = n; }
void mouse_check_double(void) { }
void MOUSE_BEGIN_DOUBLE(i16 a, i16 b, i16 c, i16 d) { (void)a; (void)b; (void)c; (void)d; }
void MOUSE_END_DOUBLE(void) { }

/* mouse_freeze -- 04bf */
void mouse_freeze(void)
{
    if (MOUSE_KNOWN_MODE) {
        moved = 0;
        frozen = -1;
    }
}

/* mouse_thaw -- 04d1 */
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

/* MOUSE_FORCE -- 0500: put the cursor at (x, y) */
void MOUSE_FORCE(i16 x, i16 y)
{
    if (MOUSE_KNOWN_MODE)
        mouse_freeze();
    else
        mouse_hide();
    MOUSE_X_TEMP = x;
    MOUSE_Y_TEMP = y;
    if (mouse_driver)
        host_mouse_warp(x, y);
    if (!MOUSE_KNOWN_MODE)
        mouse_show();
    else {
        event(x, y);
        mouse_thaw();
    }
}

/* mouse_get_status -- 057c: the cursor's position, and the buttons (the driver's, and any
   the keypad holds down) */
i16 mouse_get_status(i16 *x, i16 *y)
{
    int b = 0;
    if (mouse_driver)
        host_mouse(NULL, NULL, &b);     /* also runs the event loop: the handler may move it */
    *x = MOUSE_X_TEMP;
    *y = MOUSE_Y_TEMP;
    return (i16)(b | MOUSE_FORCE_BUTTON);
}

/* MOUSE_HORIZ_BOUND / MOUSE_VERT_BOUND -- 05bf, 05d7: the driver's limits. The host keeps
   the pointer on the 320 x 200 screen already. */
void MOUSE_HORIZ_BOUND(i16 lo, i16 hi) { (void)lo; (void)hi; }
void MOUSE_VERT_BOUND(i16 lo, i16 hi) { (void)lo; (void)hi; }

/* MOUSE_CHANGE_CURSOR_BEGIN / _END -- 05ef, 0603: around a change of the cursor image */
void MOUSE_CHANGE_CURSOR_BEGIN(void)
{
    if (MOUSE_KNOWN_MODE && MOUSE_SHOWING == 0)
        mouse_freeze();
}

void MOUSE_CHANGE_CURSOR_END(void)
{
    if (MOUSE_KNOWN_MODE && MOUSE_SHOWING == 0) {
        overlay();
        copy(screen_at, composed + image_off, rows, cols, 16, 320);
        host_vram_touched();
        mouse_thaw();
    }
}

void MOUSE_DISABLE_SCALE(void) { }

/* mouse_set_work_buffer -- 075e */
void mouse_set_work_buffer(char *bits, i16 w)
{
    work = bits;
    work_w = w;
}

/* mouse_set_view_port_loc -- 0777: the part of the screen the view port shows */
void mouse_set_view_port_loc(i16 x0, i16 y0, i16 x1, i16 y1)
{
    view_x0 = x0;
    view_y0 = y0;
    view_x1 = x1;
    view_y1 = y1;
}

/* mouse_set_view_port -- 07a8: the work-buffer pixel shown at the view port's top left */
void mouse_set_view_port(i16 x, i16 y)
{
    view_origin = work + (u16)((u16)y * (u16)work_w + (u16)x);
}

/* mouse_refresh_view_port -- 07cd: if the shown cursor overlaps the view port, draw it into
   the work buffer over the new picture, so that the next video_update shows it. Returns -1
   when it did, 0 when not. */
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

/* mouse_refresh_done -- 0903: take the cursor out of the work buffer again */
void mouse_refresh_done(void)
{
    if (saved_work)
        copy(saved_work, saved_under, saved_rows, saved_cols, 16, work_w);
}

/* MOUSE_HARD_CURSOR_MODE and MOUSE_SCREEN_SWAP serve text modes and a second screen */
void MOUSE_HARD_CURSOR_MODE(i16 mode, char *colors) { (void)mode; (void)colors; }
void MOUSE_SCREEN_SWAP(i16 mode) { (void)mode; }

/* the keypad's moves (keys.c), as KEYS_1C_HANDLER calls the event handler */
void mouse_keypad_move(i16 x, i16 y)
{
    event(x, y);
}
