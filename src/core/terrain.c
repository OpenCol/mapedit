/* terrain.c -- terrain: the table of terrain types (NAMES.TXT) and what a square is.
 *
 * A terrain byte (the map plane) holds a type in its low five bits: 0-7 open land, 8-15 and
 * 16-23 two bands of the same eight forests, 24 arctic, 25 ocean, 26 sea lane. With
 * MAP0_HILLY set the square is hills, or mountains if MAP0_MOUNTAINS is also set, and the low
 * bits are the terrain under them; terrain_type folds that into types 27 (mountains) and 28
 * (hills). terrain[] has a row of names and values for each of the 29 types.
 */
#include "msc.h"
#pragma pack(2)   /* 2-byte record packing, as in 1994: the file formats depend on it */
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

/* The terrain type of terrain byte b: 0x1b (mountains) or 0x1c (hills) for a hilly square,
 * else its low five bits. */
i16 terrain_type(i16 b)
{
    i16 r;

    if (b & 0x20)
        r = (b & 0x80) ? 0x1b : 0x1c;
    else
        r = (u8)(b & 0x1f);
    return r;
}

/* The terrain type of square (x, y); ocean (0x19) on the border ring and off the map. */
i16 terrain_at(i16 x, i16 y)
{
    i16 terrain;

    terrain = 0x19;
    if (on_map(x, y))
        terrain = terrain_type(map_get(x, y));
    return terrain;
}

/* Is the square ocean or sea lane? Reads the map without testing the coordinates, and
 * looks only at the low five bits, ignoring MAP0_HILLY. */
i16 terrain_is_ocean(i16 x, i16 y)
{
    i16 terrain;

    terrain = map_get(x, y);
    terrain = (u8)(terrain & 0x1f);
    return terrain == 0x19 || terrain == 0x1a;
}

TerrainInfo terrain[29];    /* one row of NAMES.TXT per terrain type */

/* The five OTHER_NAMES words of NAMES.TXT (string numbers); [0] is the word a forest's name
 * gets after it. */
i16 terrain_names[5];

/* Reads the next line of the open text section into terrain[n]: its name (a string number),
 * three numbers and nine values. */
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

/* Loads the terrain table from NAMES.TXT: the UNFORESTED section gives rows 0-7, FORESTED
 * rows 8-15 (each copied to the second forest band, 16-23), OTHER rows 24-28, and
 * OTHER_NAMES the five words of terrain_names. */
void load_data(void)
{
    char name[16] = "names";
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
