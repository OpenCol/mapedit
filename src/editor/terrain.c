/* Imported from mapedit-decomp src/editor/terrain by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i16 on_map(i16 x, i16 y);
extern u8 map_get(i16 x, i16 y);
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

