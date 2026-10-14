/* terrain.h -- terrain ids and the bits of the map's planes.
 *
 * The game's own constants, shared with the editor. A bit whose meaning is not known is named
 * after the bit rather than after a guess.
 */
#ifndef TERRAIN_H
#define TERRAIN_H

/* ---- terrain ids: the low five bits of a terrain byte, when MAP0_HILLY is clear ---------- */
#define TERRAIN_TUNDRA              0
#define TERRAIN_DESERT              1
#define TERRAIN_PLAINS              2
#define TERRAIN_PRAIRIE             3
#define TERRAIN_GRASSLAND           4
#define TERRAIN_SAVANNAH            5
#define TERRAIN_MARSH               6
#define TERRAIN_SWAMP               7
#define TERRAIN_BOREAL_FOREST       8       /* NAMES.TXT's "Boreal" */
#define TERRAIN_SCRUB_FOREST        9
#define TERRAIN_MIXED_FOREST        10
#define TERRAIN_BROADLEAF_FOREST    11
#define TERRAIN_CONIFER_FOREST      12
#define TERRAIN_TROPICAL_FOREST     13
#define TERRAIN_WETLAND_FOREST      14
#define TERRAIN_RAIN_FOREST         15
/* 16..23 repeat the forests of 8..15 (load_data copies their rows); what tells the two bands
 * apart in the game is not known. The editor's forest_fix moves a loaded map's second band
 * into the first. */
#define TERRAIN_BOREAL_FOREST_2     16
#define TERRAIN_SCRUB_FOREST_2      17
#define TERRAIN_MIXED_FOREST_2      18
#define TERRAIN_BROADLEAF_FOREST_2  19
#define TERRAIN_CONIFER_FOREST_2    20
#define TERRAIN_TROPICAL_FOREST_2   21
#define TERRAIN_WETLAND_FOREST_2    22
#define TERRAIN_RAIN_FOREST_2       23

/* Both bands are forest: a forest test asks for 8..23. */
#define TERRAIN_FOREST_FIRST        TERRAIN_BOREAL_FOREST     /* 8 */
#define TERRAIN_FOREST_LAST         TERRAIN_RAIN_FOREST_2     /* 23 */
#define TERRAIN_FOREST_2_FIRST      TERRAIN_BOREAL_FOREST_2   /* 16: the second band */

#define TERRAIN_ARCTIC              24
#define TERRAIN_OCEAN               25
#define TERRAIN_SEA_LANE            26
#define TERRAIN_MOUNTAINS           27      /* never in the five bits: MAP0_HILLY */
#define TERRAIN_HILLS               28
#define TERRAIN_COUNT               29

/* ---- plane 0 (map): the terrain byte; terrain_type turns it into a terrain id ------------ */
#define MAP0_TERRAIN_MASK   0x1f    /* a terrain id only when MAP0_HILLY is clear */
#define MAP0_HILLY          0x20    /* hills or mountains; the low five bits are then not an id */
#define MAP0_RIVER          0x40    /* a river */
#define MAP0_MOUNTAINS      0x80    /* with MAP0_HILLY: mountains rather than hills */
#define MAP0_MAJOR_RIVER    0x80    /* the same bit with MAP0_RIVER: a major river, not a minor */

/* ---- plane 1 (feature): a bitfield (feature_get, feature_loc) --------------------------- */
#define MAP1_UNIT           0x01    /* a unit stands here */
#define MAP1_SETTLEMENT     0x02    /* a colony or a village, which also counts as a road */
#define MAP1_DEPLETED       0x04    /* the square's resource is used up (resource_at) */
#define MAP1_ROAD           0x08    /* a road */
#define MAP1_CLAIM_SETTLED  0x10    /* the land has been paid for or taken from its natives */
#define MAP1_BIT5           0x20    /* meaning not known */
#define MAP1_PLOWED         0x40    /* plowed */

/* ---- plane 2 (continent): the region number in the low nibble, the owner in the high ---- */
#define MAP2_NIBBLE_NONE    15      /* no owner: owner_of returns -1 for it */

/* ---- plane 3 (site): the high nibble has a bit per European nation that has seen it ----- */
#define MAP3_NATION_BIT(n)  ((1 << (n)) << 4)

#endif
