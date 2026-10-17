/* checks.c -- the checks before saving (port: options.h, save_checks).
 *
 * The game does not play a map from the editor exactly as it was drawn. When it starts a game
 * on one, it makes the two outermost columns on each side sea lane and the top and bottom rows
 * arctic, numbers the continents again and clears the feature plane. The editor's help adds
 * that rivers must be at least two squares long, and that more than 15 land masses (or seas)
 * may confuse the computer players. Before a save, save_checks_pass runs Find Continents (so
 * the saved map has its continents numbered, as the help asks) and looks for:
 *   - a map with no land;
 *   - more than 15 land masses, or more than 15 seas (Find Continents's two warnings);
 *   - land in the columns next to the left and right edges, which the game makes sea lane;
 *   - a river square with no river in any of the eight squares around it.
 * What it finds is listed in one popup that asks whether to save anyway.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "checks.h"
#include "game.h"
#include "map.h"
#include "mem.h"
#include "options.h"
#include "popup.h"
#include "terrain.h"

/* land: anything but ocean and sea lane (hills and mountains are always land) */
static i16 is_land(u8 b)
{
    return (b & MAP0_HILLY) || (b & MAP0_TERRAIN_MASK) < TERRAIN_OCEAN;
}

static i16 has_river(i16 x, i16 y)
{
    return x >= 0 && y >= 0 && x < map_file.w && y < map_file.h && (map_get(x, y) & MAP0_RIVER);
}

i16 save_checks_pass(void)
{
    static char lines[6][0x80];
    char *text[8];
    static char *items[] = {"Save anyway", "Do not save", 0};
    i16 n = 0, land = 0, edge = 0, lone = 0, flags = 0, x, y;
    u8 b;

    if (!editor_options.save_checks)
        return 1;
    sprite_base = mem_get(0xfde8L);
    if (sprite_base != 0) {
        flags = map_find_continents();
        mem_free(sprite_base);
        sprite_base = 0;
    }
    for (y = 1; y < map_file.h - 1; y++)
        for (x = 1; x < map_file.w - 1; x++) {
            b = map_get(x, y);
            if (is_land(b)) {
                land++;
                if (x == 1 || x == map_file.w - 2)
                    edge++;
            }
            if ((b & MAP0_RIVER) && !has_river(x - 1, y - 1) && !has_river(x, y - 1) && !has_river(x + 1, y - 1)
                && !has_river(x - 1, y) && !has_river(x + 1, y) && !has_river(x - 1, y + 1)
                && !has_river(x, y + 1) && !has_river(x + 1, y + 1))
                lone++;
        }
    text[n++] = "^^{Before saving}";
    if (land == 0)
        text[n++] = "^The map has no land.";
    if (flags & 1)
        text[n] = "^More than 15 land masses: the computer players may find their way less well.", n++;
    if (flags & 2)
        text[n] = "^More than 15 seas: the computer players may find their way less well.", n++;
    if (edge) {
        msc_sprintf(lines[n], "^%d land square%s in the columns next to the left and right edges: the game "
                    "makes those columns sea lane.", edge, edge == 1 ? "" : "s");
        text[n] = lines[n], n++;
    }
    if (lone) {
        msc_sprintf(lines[n], "^%d river square%s with no river next to %s: a river must be at least two "
                    "squares long.", lone, lone == 1 ? "" : "s", lone == 1 ? "it" : "them");
        text[n] = lines[n], n++;
    }
    if (n == 1)
        return 1;
    text[n++] = "";
    text[n] = 0;
    return popup_lines(220, text, items, 2) == 1;
}
