/* tilepicker.c -- Map Tile Select: the palette of terrains and features to paint with. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "menu.h"
#include "buffer.h"
#include "edit.h"
#include "error.h"
#include "hspot.h"
#include "infopanel.h"
#include "keys.h"
#include "map.h"
#include "mapview.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "screen.h"
#include "sprite.h"
#include "tilepicker.h"

/* add_item -- 0000:0d2a, 116 bytes */
i16 myspot = 1;                                                 /* DS:0x01ce (defined here: lands after info_window_draw's "Coast Protect: OFF") */

/* A 16x16 icon on the selection screen with its hot spot (id 0x13, values 3 and n); the
 * selected one is boxed in white and remembered in myspot. */
void add_item(i16 x, i16 y, i16 icon, i16 n, i16 sel)
{
    sprite_draw(icon, tiles, &scr_work, x, y);
    hspot_add(x, y, x + 0xf, y + 0xf, 3, n, 0x13);
    if (sel) {
        myspot = numspots;
        buffer_draw_box(x - 1, y - 1, x + 0x10, scr_work, y + 0x10, 0xf);
    }
}

/* add_item_2 -- 0000:0d9e, 118 bytes */
/* The same for a terrain tile. */
void add_item_2(i16 x, i16 y, i16 tile, i16 n, i16 sel)
{
    tile_draw(terrain_1, tile, &scr_work, x, y);
    hspot_add(x, y, x + 0xf, y + 0xf, 3, n, 0x13);
    if (sel) {
        myspot = numspots;
        buffer_draw_box(x - 1, y - 1, x + 0x10, scr_work, y + 0x10, 0xf);
    }
}

/* selection_screen_display -- 0000:0e14, 696 bytes */
/* The terrain selection screen: two rows of eight terrain tiles (the second row forested,
 * hot spot values row/column), then ocean, sea lane, lakes and the four features (values 3
 * and 0-6), the selected one boxed; the selection's name at (0xa0, 0xa). */
void selection_screen_display(void)
{
    i16 r;
    i16 x;
    i16 y;
    i16 j;
    i16 base;
    i16 mark;
    i16 tile;
    i16 hilite;

    buffer_fill(0, scr_work);
    numspots = 0;
    for (r = 0; r < 2; r++) {
        for (j = 0; j < 8; j++) {
            x = j * 0x11 + 1;
            y = r * 0x11 + 1;
            switch (r) {
            case 0:
                mark = 0;
                base = 0;
                tile = j;
                break;
            default:
                mark = 0;
                base = 8;
                tile = j;
                if (j == 1)
                    tile = 0x11;
                break;
            }
            tile_draw(terrain_1, tile, &scr_work, x, y);
            if (r != 0 && j != 1)
                sprite_draw(0x41, tiles, &scr_work, x, y);
            hspot_add(x, y, x + 0xf, y + 0xf, r, j, 0x13);
            if ((char)(base + j) == selected_terrain && selected_physical_or_mask == 0)
                buffer_draw_box(x - 1, y - 1, x + 0x10, scr_work, y + 0x10, 0xf);
        }
    }
    x = 1;
    y = 0x30;
    if (selected_terrain == 0x18 && selected_physical_or_mask == 0)
        hilite = 1;
    else
        hilite = 0;
    add_item_2(x, y, 0x18, 0, hilite);
    add_item_2(x += 0x11, y, 0x19, 1, selected_terrain == 0x19 ? 1 : 0);
    add_item_2(x += 0x11, y, 0x1a, 2, selected_terrain == 0x1a ? 1 : 0);
    add_item(x += 0x11, y, 4, 3, selected_physical_or_mask == 0xc0 ? 1 : 0);
    add_item(x += 0x11, y, 0x14, 4, selected_physical_or_mask == 0x40 ? 1 : 0);
    add_item(x += 0x11, y, 0x24, 5, selected_physical_or_mask == 0xa0 ? 1 : 0);
    add_item(x += 0x11, y, 0x34, 6, selected_physical_or_mask == 0x20 ? 1 : 0);
    x = 0xa0;
    y = 0xa;
    if (selected_physical_or_mask & 0x20) {
        if (selected_physical_or_mask & 0x80)
            info_terrain(0x1b, &x, &y);
        else
            info_terrain(0x1c, &x, &y);
    } else if (selected_physical_or_mask & 0x40) {
        if (selected_physical_or_mask & 0x80)
            info_terrain_2(2, &x, &y);
        else
            info_terrain_2(3, &x, &y);
    } else
        info_terrain(selected_terrain, &x, &y);
    mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
}

/* parse_spot -- 0000:10cc, 290 bytes */
/* A hot spot hit on the selection screen: rows 0-2 pick a terrain (column, +8 forested,
 * +0x10), value 3 picks ocean, sea lane, lakes or a feature (and/or masks on the map byte,
 * right-mouse change for the features). Redraws when the selection changed. */
void parse_spot(i16 n)
{
    i16 clr;
    i16 obits;
    i16 v1;
    i16 v2;
    i16 terr;
    i16 rm;

    if (n == 0)
        return;
    v2 = spot[n].v2;
    v1 = spot[n].v1;
    if (v1 < 3) {
        terr = (u8)v2;
        clr = 0xff;
        obits = 0;
        rm = 0;
        switch (v1) {
        case 0:
            break;
        case 1:
            terr += 8;
            break;
        default:
            terr += 0x10;
            break;
        }
    } else {
        switch (v2) {
        case 0:
            terr = 0x18;
            clr = 0xff;
            obits = 0;
            rm = 0;
            break;
        case 1:
            terr = 0x19;
            clr = 0x40;
            obits = 0;
            rm = 0;
            break;
        case 2:
            terr = 0x1a;
            clr = 0x40;
            obits = 0;
            rm = 0;
            break;
        case 3:
            terr = 0;
            clr = ~0xe0;
            obits = 0xc0;
            rm = 1;
            break;
        case 4:
            terr = 0;
            clr = ~0xc0;
            obits = 0x40;
            rm = 1;
            break;
        case 5:
            terr = 0;
            clr = ~0xe0;
            obits = 0xa0;
            rm = 1;
            break;
        case 6:
            terr = 0;
            clr = ~0xa0;
            obits = 0x20;
            rm = 1;
            break;
        }
    }
    if ((char)terr != selected_terrain || selected_physical_and_mask != (u8)clr
        || selected_physical_or_mask != (u8)obits || selected_right_mouse_change != (char)rm) {
        selected_terrain = terr;
        selected_physical_and_mask = clr;
        selected_physical_or_mask = obits;
        selected_right_mouse_change = rm;
        selection_screen_display();
    }
}

/* A mouse stroke on the selection screen; 0 once the stroke ended. */
i16 selection_mouse(void)
{
    i16 running;
    i16 a;

    running = 1;
    if (mouse_any_stroke)
        parse_spot(hspot_which(mouse_x, mouse_y, 0x13));
    if (mouse_stop_stroke)
        running = 0;
    return running;
}

/* selection_screen -- 0000:1226, 234 bytes */
/* The terrain selection screen's event loop: the arrow keys (or keypad 8/4/6/2) move the
 * selection around the 3 rows of 8, the mouse picks, any other key leaves. */
void selection_screen(void)
{
    i16 running;
    i16 move;
    i16 key;

    running = 1;
    selection_screen_display();
    mouse_init_cycle();
    do {
        mouse_begin_cycle(0);
        if (keys_any()) {
            move = 0;
            key = keys_get();
            switch (key) {
            case 0x14d:
                move = 1;
                break;
            case 0x150:
                move = 8;
                if (myspot == 0x10)
                    move = -8;
                break;
            case 0x148:
                move = -8;
                if (myspot == 8)
                    move = 8;
                break;
            case 0x14b:
                move = -1;
                break;
            default:
                running = 0;
                break;
            }
            if (move) {
                myspot += move - 1;
                if (myspot == 0x17)
                    myspot = 0;
                if (myspot > 0x17)
                    myspot -= 0x18;
                if (myspot < 0)
                    myspot += 8;
                myspot++;
                parse_spot(myspot);
            }
        }
        if (mouse_any_stroke)
            running = selection_mouse();
        mouse_end_cycle(0, running);
    } while (running);
    main_screen_refresh();
}
