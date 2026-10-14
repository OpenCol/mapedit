/* editstate.c -- the editing state: the cursor, the selection, the brush, coasts, undo.
 *
 * The views show this state and the commands change it: the cursor square, the selected
 * terrain and how it paints, the brush size, coastline protection, whether the map has
 * unsaved changes, and the one level of undo.
 *
 * Painting a square (edit.c) works on its terrain byte: the high three bits (hills or
 * mountains, river) are ANDed with selected_physical_and_mask and ORed with
 * selected_physical_or_mask, and the low five bits become selected_terrain. A terrain
 * selection sets the type and keeps the overlays (ocean and sea lane keep only a river); an
 * overlay selection (hills, mountains, a minor or major river) sets the bits and keeps the
 * type. Picking up a square (the right button) makes its terrain the selection.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
#include "font.h"
#include "loader.h"
#include "editstate.h"
#include "error.h"
#include "map.h"
#include "pack.h"
#include "pal.h"
#include "sprite.h"

i16 map_changes = 0;        /* the map has changed since it was loaded or saved */

i16 small_map_needs_update = 0;     /* unused */

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

i16 map_undo_ems_handle = -1;       /* unused */

i16 cursor_x;               /* the cursor square */

i16 cursor_y;

i16 cursor_on;              /* the blinking cursor square is drawn at the moment */

/* Takes the undo snapshot before an edit: copies the terrain plane, all 12000 bytes of it,
 * into the undo buffer. Only the terrain is saved; features and continents are not. */
void make_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_available) {
        msc__fmemcpy(map_undo_memory, map, 0x2ee0);
        undo_active = 1;
    }
}

/* Restores the terrain plane from the last snapshot. The snapshot stays, so undoing twice
 * restores the same state again. */
void perform_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_active)
        msc__fmemcpy(map, map_undo_memory, 0x2ee0);
}
