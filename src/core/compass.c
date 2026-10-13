/* From mapedit-decomp src/editor/compass (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
#include "map.h"
/* MODULE compass: MAPEDIT.EXE data only (compass.obj), data at DS:0x06b4.
 *
 * No code: the six direction tables map_a.c's generator walks (card_x/y: the four cardinal
 * steps; move_x/y: the eight neighbours clockwise from north; radius_x/y: the 20 squares
 * within two steps, nearest first). Each table carries two trailing zero bytes as shipped.
 * The 76 bytes land at DS:0x06b4-0x0700, between vicemisc.obj's literals and map_6.obj's.
 */

i8 card_x[6] = { 0, 1, 0, -1, 0, 0 };                                  /* DS:0x06b4 */
i8 card_y[6] = { -1, 0, 1, 0, 0, 0 };                                  /* DS:0x06ba */
i8 move_x[10] = { 0, 1, 1, 1, 0, -1, -1, -1, 0, 0 };                   /* DS:0x06c0 */
i8 move_y[10] = { -1, -1, 0, 1, 1, 1, 0, -1, 0, 0 };                   /* DS:0x06ca */
i8 radius_x[22] = { 0, 1, 0, -1, -1, 1, 1, -1, 0, 2, 0, -2, -1, 1, -1, 1, -2, -2, 2, 2, 0, 0 };   /* DS:0x06d4 */
i8 radius_y[22] = { -1, 0, 1, 0, -1, -1, 1, 1, -2, 0, 2, 0, -2, -2, 2, 2, -1, 1, -1, 1, 0, 0 };   /* DS:0x06ea */

