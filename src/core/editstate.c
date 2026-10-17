/* editstate.c -- the editing state: the cursor, the selection, the brush, coasts, undo.
 *
 * The views show this state and the commands change it: the cursor square, the selected
 * terrain and how it paints, the brush size, coastline protection, whether the map has
 * unsaved changes, and the one level of undo (or, with the port's option, the undo history).
 *
 * Painting a square (edit.c) works on its terrain byte: the high three bits (hills or
 * mountains, river) are ANDed with selected_physical_and_mask and ORed with
 * selected_physical_or_mask, and the low five bits become selected_terrain. A terrain
 * selection sets the type and keeps the overlays (ocean and sea lane keep only a river); an
 * overlay selection (hills, mountains, a minor or major river) sets the bits and keeps the
 * type. Picking up a square (the right button) makes its terrain the selection.
 */
#include <stdlib.h>
#include <string.h>

#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "loader.h"
#include "editstate.h"
#include "error.h"
#include "map.h"
#include "options.h"
#include "pack.h"
#include "pal.h"
#include "sprite.h"

i16 map_changes = 0;        /* the map has changed since it was loaded or saved */

i16 fill_radius = 0;        /* the brush: paints a (2r + 1)-square block, r = 0-2 */

i16 map_selected = 0;       /* the command line named the map to edit (M:file) */

i16 coastline_protect = 0;  /* painting leaves ocean and sea-lane squares alone */

i16 undo_available = 0;     /* the undo buffer was allocated at startup */

i16 undo_active = 0;        /* the undo buffer holds a snapshot */

i16 create_me_now = 0;      /* start with a new map (C on the command line, or no file chosen) */

/* The terrain type the brush paints, ocean at first. (A negative value would keep each
 * square's type; no selection sets one.) */
char selected_terrain = 0x19;

u8 selected_physical_and_mask = 0x40;    /* the high bits a painted square keeps */

u8 selected_physical_or_mask = 0;        /* the high bits painting sets */

/* The selection is an overlay (hills, mountains, a river): painting keeps each square's type,
 * and picking up removes the overlay instead. */
char selected_right_mouse_change = 0;

void *map_undo_memory = 0;  /* the undo buffer: a copy of the terrain plane */

i16 cursor_x;               /* the cursor square */

i16 cursor_y;

i16 cursor_on;              /* the blinking cursor square is drawn at the moment */

/* Port: the undo history (options.h, undo_levels), in place of the snapshot when the option
 * is on. Each step is a copy of the three planes a .MP file holds, 12000 bytes each.
 * make_undo keeps the map as it finds it in `pending`; that copy becomes a step only once the
 * map is seen to differ from it (at the next make_undo, Undo or Redo), so the strokes that
 * only move the view leave no steps. A new step clears the redo steps; when the history is
 * full the oldest step is dropped. */
#define UNDO_STEP (3 * 12000L)

typedef struct { u8 **step; i16 n; } UndoStack;

static UndoStack undo_back;     /* the steps Undo goes back to, the newest last */
static UndoStack undo_fwd;      /* the steps Redo goes forward to, the nearest last */
static u8 *pending;             /* the map at the last make_undo */
static i16 pending_valid;

/* copies the three planes into buf (a new block when buf is 0); 0 when out of memory */
static u8 *undo_take(u8 *buf)
{
    u16 n = (u16)map_size;

    if (buf == 0)
        buf = malloc(UNDO_STEP);
    if (buf != 0) {
        memcpy(buf, map, n);
        memcpy(buf + n, feature, n);
        memcpy(buf + 2 * n, continent, n);
    }
    return buf;
}

static void undo_put_back(const u8 *buf)
{
    u16 n = (u16)map_size;

    memcpy(map, buf, n);
    memcpy(feature, buf + n, n);
    memcpy(continent, buf + 2 * n, n);
}

static i16 undo_same(const u8 *buf)
{
    u16 n = (u16)map_size;

    return !memcmp(map, buf, n) && !memcmp(feature, buf + n, n) && !memcmp(continent, buf + 2 * n, n);
}

static void undo_push(UndoStack *s, u8 *buf)
{
    if (s->step == 0)
        s->step = calloc((size_t)editor_options.undo_levels, sizeof *s->step);
    if (s->step == 0) {
        free(buf);
        return;
    }
    if (s->n == editor_options.undo_levels) {
        free(s->step[0]);
        memmove(s->step, s->step + 1, (size_t)(s->n - 1) * sizeof *s->step);
        s->n--;
    }
    s->step[s->n++] = buf;
}

static void undo_clear(UndoStack *s)
{
    while (s->n > 0)
        free(s->step[--s->n]);
}

/* the map has changed since make_undo: what it found becomes a step */
static void undo_commit(void)
{
    if (pending_valid && !undo_same(pending)) {
        undo_push(&undo_back, pending);
        pending = 0;
        undo_clear(&undo_fwd);
    }
    pending_valid = 0;
}

/* Port: one step of Undo (redo 0) or Redo (redo 1) through the history. Returns 1 when the map
 * changed, 0 when there was no step to take. */
i16 undo_step(i16 redo)
{
    UndoStack *from = redo ? &undo_fwd : &undo_back;
    UndoStack *to = redo ? &undo_back : &undo_fwd;
    u8 *now;

    undo_commit();
    if (from->n == 0)
        return 0;
    now = undo_take(0);
    if (now != 0)
        undo_push(to, now);
    from->n--;
    undo_put_back(from->step[from->n]);
    free(from->step[from->n]);
    return 1;
}

/* Port: a map was loaded or created: the history belongs to the one before it. Does nothing
 * without the history (the original's snapshot outlives the map it was taken of). */
void undo_reset(void)
{
    if (editor_options.undo_levels == 0)
        return;
    undo_clear(&undo_back);
    undo_clear(&undo_fwd);
    pending_valid = 0;
}

/* Takes the undo snapshot before an edit: copies the terrain plane, all 12000 bytes of it,
 * into the undo buffer. Only the terrain is saved; features and continents are not.
 * Port: with the history (undo_levels), keeps the map as it is instead, for undo_commit. */
void make_undo(void)
{

    if (editor_options.undo_levels) {
        undo_commit();
        pending = undo_take(pending);
        pending_valid = pending != 0;
        return;
    }
    if (undo_available) {
        msc__fmemcpy(map_undo_memory, map, 0x2ee0);
        undo_active = 1;
    }
}

/* Restores the terrain plane from the last snapshot. The snapshot stays, so undoing twice
 * restores the same state again. */
void perform_undo(void)
{

    if (undo_active)
        msc__fmemcpy(map, map_undo_memory, 0x2ee0);
}
