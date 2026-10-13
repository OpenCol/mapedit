/* assets.c -- the art the editor loads: its sprite series and tiles, and the screens it draws into. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "assets.h"
#include "error.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"

void *cursor = 0;                           /* DS:0x0064 */

void *icons = 0;                            /* DS:0x0068 */

void *buildings = 0;                        /* DS:0x006c */

DosPort *main_tile = 0;                         /* DS:0x0090 */

DosPort scr_back;                         /* DS:0x49fa */

DosPort scr_slide;                        /* DS:0x4b4a */

void *load_terrain_series(char *name)
{
    DosPort port;
    char *tiles;
    void *sheet;
    void *r;
    i16 n;
    i16 i;

    r = 0;
    n = 12;
    tiles = tile_allocate(n);
    if (tiles == 0) {
        exit_value = 0x321;
        goto out;
    }
    sheet = sprite_series_load(name, 0x4000);
    if (sheet == 0) {
        exit_value = 0x322;
        goto out;
    }
    port.h = port.w = 16;
    for (i = 0; i < n; i++) {
        port.bits = tiles + i * 0x100;
        sprite_draw(i + 1, sheet, &port, 0, 0);
    }
    mem_free(sheet);
    r = tiles;
out:
    return r;
}

i16 load_terrain_tiles(void)
{
    i16 r;

    r = 1;
    if ((terrain_1 = load_terrain_series("terrain")) != 0)
        r = 0;
    return r;
}
