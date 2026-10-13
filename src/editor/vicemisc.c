/* From mapedit-decomp src/editor/vicemisc (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "pack.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "sprite.h"
#include "text.h"
#include "units.h"
/* MODULE vicemisc: MAPEDIT.EXE segment 08f9 (vicemisc.obj), file 0xa594-0xb176, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn construct_main_menu 0x0a594
 * fn load_terrain_series 0x0b09e
 * fn load_terrain_tiles  0x0b152
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl26_72090). CENSUS: the layout cuts construct_main_menu at 2274 bytes and
 * files the next 552 bytes (0xae76-0xb09e) under `__acrtused`; that public is MSC's absolute
 * link marker for the module holding main (not code), and 0xae76 is the middle of
 * construct_main_menu (`add sp,0xc; push 0; push ds; push "" ...`, and its first `jmp out`
 * lands at +0xaff). construct_main_menu is one 2826-byte function, 0xa594-0xb09e.
 *
 * THE EDITOR'S MAIN MENU BAR. construct_main_menu() builds menu_bar from MENU.TXT: seven
 * submenus (game, view, orders, reports, trade, the Continental Congress "cup", pedia),
 * each item's text the next line of its section, with "" separators. The "cup" menu's
 * debug item is hidden unless the cheat bit (game_flags 0x2000) is set. It returns 0, or 1
 * when the menu or a section cannot be had. The call sequence and every item id are
 * VICEROY's create_main_menu's, one to one; text_get() takes no argument (it returns the
 * section's next line). "elg": the returned item's offset lives in di, its segment in bp-2.
 * (The editor's own menu handling is in mapedit.obj; the game's item ids are kept.)
 *
 * load_terrain_series(name) draws the 12 16x16 terrain tiles of an icon sheet into a tile
 * sheet (tile_allocate; exit_value 0x321/0x322 on failure); load_terrain_tiles loads
 * "terrain" into terrain_1. Both still compile under "elg" (the induction variable di walks
 * the tile sheet), as in VICEROY.
 */

#include "records.h"

/* load_terrain_series -- 08f9:0b0e, 180 bytes */

/* load_terrain_tiles -- 08f9:0bc2, 36 bytes */

