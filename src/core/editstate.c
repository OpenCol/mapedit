/* editstate.c -- the editing state the views show and the commands change: the cursor, the selected terrain and how it paints, the brush, coastline protection, whether the map changed, and the one level of undo. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
#include "editstate.h"
#include "error.h"
#include "map.h"
#include "pack.h"
#include "pal.h"
#include "sprite.h"

i16 map_changes = 0;                            /* DS:0x0046 */

i16 small_map_needs_update = 0;                 /* DS:0x0048 */

i16 fill_radius = 0;                            /* DS:0x004a */

i16 map_selected = 0;                           /* DS:0x004c */

i16 coastline_protect = 0;                      /* DS:0x004e */

i16 undo_available = 0;                         /* DS:0x0050 */

i16 undo_active = 0;                            /* DS:0x0052 */

i16 create_me_now = 0;                          /* DS:0x0054 */

/* 11 unused declarations before selected_terrain and 6 after it set its own position (mod 17)
 * for parse_spot's `cmp [bp-0xa],al` without moving any other symbol. */
char selected_terrain = 0x19;                   /* DS:0x0056 */

u8 selected_physical_and_mask = 0x40;/* DS:0x0057 */

u8 selected_physical_or_mask = 0;    /* DS:0x0058 */

char selected_right_mouse_change = 0;           /* DS:0x0059 */

void *map_undo_memory = 0;                  /* DS:0x005a */

i16 map_undo_ems_handle = -1;                   /* DS:0x005e */

i16 cursor_x;                             /* DS:0x4b52 */

i16 cursor_y;                             /* DS:0x4b54 */

i16 cursor_on;                            /* DS:0x6972 */

/* Copies the terrain plane (0x2ee0 bytes) into the undo buffer. Both have a dead
 * `result = -1` (the `mov ax,-1`) and 6 more bytes of unused frame. */
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

/* perform_undo -- 0000:077e, 40 bytes */
void perform_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_active)
        msc__fmemcpy(map, map_undo_memory, 0x2ee0);
}
