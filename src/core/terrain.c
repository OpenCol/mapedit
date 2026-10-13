/* From mapedit-decomp src/editor/terrain (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "error.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "text.h"
#include "theme.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "units.h"
/* MODULE terrain: MAPEDIT.EXE segment 09b7 (terrain.obj), file 0xb176-0xb242, 4 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn terrain_type      0x0b176
 * fn terrain_at        0x0b1a2
 * fn terrain_is_ocean  0x0b1dc
 * fn terrain_is_forest 0x0b20a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_03e4).
 *
 * TERRAIN TESTS (root code, frame 03e4). Windows twins: `terrain_at`,
 * terrain_is_ocean.c.
 *   - terrain_type(b): a map byte with bit 0x20 set is terrain 0x1b (bit 0x80 also set) or
 *     0x1c; otherwise the type is b & 0x1f.
 *   - terrain_at(x, y) is 0x19 (ocean) off the map.
 *   - terrain_is_ocean: ocean or sea lane. DOS reads the map without the Windows on_map test.
 *   - terrain_is_forest: whether the square's type is in 8..23.
 */

/* terrain_type -- 09b7:0006, 44 bytes */
i16 terrain_type(i16 b)
{
    i16 r;

    if (b & 0x20)
        r = (b & 0x80) ? 0x1b : 0x1c;
    else
        r = (u8)(b & 0x1f);
    return r;
}
/* terrain_at -- 09b7:0032, 58 bytes */
i16 terrain_at(i16 x, i16 y)
{
    i16 terrain;

    terrain = 0x19;
    if (on_map(x, y))
        terrain = terrain_type(map_get(x, y));
    return terrain;
}
/* terrain_is_ocean -- 09b7:006c, 46 bytes */
i16 terrain_is_ocean(i16 x, i16 y)
{
    i16 terrain;

    terrain = map_get(x, y);
    terrain = (u8)(terrain & 0x1f);
    return terrain == 0x19 || terrain == 0x1a;
}
/* terrain_is_forest -- 09b7:009a, 56 bytes */
i16 terrain_is_forest(i16 x, i16 y)
{
    i16 terrain;

    terrain = map_get(x, y);
    terrain = (u8)(terrain & 0x1f);
    return (terrain >= 8 && terrain < 0x10) || (terrain >= 0x10 && terrain < 0x18);
}

TerrainInfo terrain[29];                   /* DS:0x4ee6: 464 bytes, NAMES.TXT's terrain rows */

i16 terrain_names[5];                     /* DS:0x4b5a: five word numbers */

/* One terrain row from the text file: its name word, three numbers and nine values. */
void load_terrain(register i16 n)
{
    register i16 i;

    text_get();
    terrain[n].name = text_item_string();
    terrain[n].f2 = text_item_number();
    terrain[n].f3 = text_item_number();
    terrain[n].f4 = text_item_number();
    for (i = 0; i < 9; i++)
        terrain[n].v[i] = text_item_number();
}

/* load_data -- 0000:2336, 324 bytes */
/* The terrain table from NAMES.TXT: 8 unforested rows, 8 forested (copied to rows 16-23),
 * 5 others, the 5 feature words, and the editor's colours from COLORS. */
void load_data(void)
{
    char name[16] = "names";
    char labels[16] = "labels";
    i16 i;
    TerrainInfo *p;
    i16 *q;

    text_open(name, "UNFORESTED");
    for (i = 0; i < 8; i++)
        load_terrain(i);
    text_open(0, "FORESTED");
    i = 0;
    for (p = &terrain[16]; p < &terrain[24]; p++) {
        load_terrain(i + 8);
        *p = *(p - 8);
        i++;
    }
    text_open(0, "OTHER");
    for (i = 0; i < 5; i++)
        load_terrain(i + 0x18);
    text_open(0, "OTHER_NAMES");
    for (q = terrain_names; q < &terrain_names[5]; ) {
        text_get();
        *q++ = text_item_string();
    }
}
