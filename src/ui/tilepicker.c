/* tilepicker.c -- Map Tile Select: the palette of terrains and features to paint with.
 *
 * The screen replaces the whole work screen with three rows of 16 x 16 pictures, each a hot
 * spot (id 0x13): the eight unforested terrains, the eight forested ones, then Arctic, Ocean,
 * Sea Lane and the four features (major river, minor river, mountains, hills). The current
 * selection is boxed in colour 15 and named at the top right. Picking one sets the editing
 * state the paint commands use (editstate.c):
 *   selected_terrain              the terrain painted (0 for a feature);
 *   selected_physical_and_mask    which of the square's feature bits painting keeps;
 *   selected_physical_or_mask     the feature bits painting adds (0 for a terrain);
 *   selected_right_mouse_change   set for a feature: painting changes only the feature bits.
 * The mouse picks a spot and leaves the screen when the button is released; the arrow keys
 * move the selection; any other key leaves.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "editstate.h"
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

i16 myspot = 1;                         /* the hot spot the arrow keys move from (1-based) */

/* Draws sprite `icon` of the PHYS0 series at (x, y) with a hot spot over it (values 3 and n);
 * when sel is set it is boxed and becomes myspot. Only the bottom row goes through add_item and
 * add_item_2, so only a selection there updates myspot; a terrain picked in the two upper rows
 * with the mouse leaves myspot where it was, as in the original. */
void add_item(i16 x, i16 y, i16 icon, i16 n, i16 sel)
{
    sprite_draw(icon, tiles, &scr_work, x, y);
    hspot_add(x, y, x + 0xf, y + 0xf, 3, n, 0x13);
    if (sel) {
        myspot = numspots;
        buffer_draw_box(x - 1, y - 1, x + 0x10, scr_work, y + 0x10, 0xf);
    }
}

/* The same for terrain tile `tile`. */
void add_item_2(i16 x, i16 y, i16 tile, i16 n, i16 sel)
{
    tile_draw(terrain_1, tile, &scr_work, x, y);
    hspot_add(x, y, x + 0xf, y + 0xf, 3, n, 0x13);
    if (sel) {
        myspot = numspots;
        buffer_draw_box(x - 1, y - 1, x + 0x10, scr_work, y + 0x10, 0xf);
    }
}

/* Draws the selection screen and puts it on the video screen. The upper rows' hot spots carry
 * the row (0 or 1) and the column; a forested tile gets the forest sprite drawn over it,
 * except Scrub (column 1), whose tile already shows its trees. The bottom row's spots carry 3
 * and 0-6. The selection's name is written at (160, 10). */
void selection_screen_display(void)
{
    i16 r;
    i16 x;
    i16 y;
    i16 j;
    i16 base;
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
                base = 0;
                tile = j;
                break;
            default:
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

/* Selects what hot spot n stands for (0, no spot, does nothing). Rows 0 and 1 select terrain
 * column or column + 8; a row 2 would add 0x10, but the screen has none. The bottom row
 * selects Arctic, Ocean or Sea Lane, or a feature. The masks:
 *   a terrain or Arctic   keeps every feature bit;
 *   Ocean, Sea Lane       keep only a river (0x40);
 *   major river clears all three bits and sets 0xc0;  minor river clears 0xc0, sets 0x40;
 *   mountains   clears all three bits and sets 0xa0;  hills clears 0xa0, sets 0x20.
 * The screen is redrawn only when the selection changed. */
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

/* Handles a mouse stroke on the selection screen: selects the spot under the mouse. Returns
 * 0 once the stroke has ended (the screen then closes), else 1. */
i16 selection_mouse(void)
{
    i16 running;

    running = 1;
    if (mouse_any_stroke)
        parse_spot(hspot_which(mouse_x, mouse_y, 0x13));
    if (mouse_stop_stroke)
        running = 0;
    return running;
}

/* Runs Map Tile Select until a key other than an arrow, or the end of a mouse stroke, and
 * then redraws the main screen. The arrows move myspot over the 23 spots, numbered row by row
 * (8, 8, 7), and select the spot they land on. Right and left run on through the rows and
 * wrap from the last spot to the first, but left from the first spot goes to the end of the
 * top row. Down wraps from the bottom row to the top; down from the last spot of the middle
 * row, which has nothing below it, goes up instead. Up from the top row stays put, except
 * from its last spot, which goes down to the middle row. */
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
