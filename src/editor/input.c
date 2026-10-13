/* input.c -- the editor's input: the main loop, the keys, mouse strokes over the map and the minimap. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "editstate.h"
#include "font.h"
#include "edit.h"
#include "error.h"
#include "game.h"
#include "infopanel.h"
#include "input.h"
#include "keys.h"
#include "machine.h"
#include "map.h"
#include "mapedit.h"
#include "mapview.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "tilepicker.h"
#include "timer.h"

i32 game_clock;                          /* DS:0x4a14 */

i16 key_parsed;                           /* DS:0x4a68 */

i16 mykey;                                /* DS:0x526c */

i16 mouse_stroke_type;                    /* DS:0x4e8e */

i16 mouse_current_area;                   /* DS:0x5ace */

i32 cursor_clock;                        /* DS:0x4ea0 */

/* Waits for a key or a mouse stroke; returns the key (0 for the mouse). */
i16 event_wait(void)
{
    i16 running;
    i32 now;
    i16 key;
    i32 time;

    running = 1;
    key = 0;
    timer_read();
    mouse_init_cycle();
    do {
        mouse_begin_cycle(0);
        if (keys_any()) {
            key = keys_get();
            running = 0;
        }
        if (mouse_stop_stroke)
            running = 0;
        mouse_end_cycle(0, running);
    } while (running);
    return key;
}

i16 shift_key(void)
{
    return *(u8 *)(msc_bda + 0x0017) & 3;
}

/* Which screen area the mouse is in: 2 the minimap (0xfc, 9)-(0x134, 0x30), 3 the info window
 * (0xf1, 0x32)-(0x140, 0xc8), 1 the map (0, 8)-(map_pixel_size), else 0. The mouse position
 * is read straight from the globals ("e" caches them in bx/cx); locals give cmp reg,mem. */
i16 mouse_area(void)
{
    i16 r;

    r = 0;
    if (mouse_x >= 0xfc && mouse_y >= 9 && mouse_x < 0x134 && mouse_y < 0x30)
        r = 2;
    if (mouse_x >= 0xf1 && mouse_y >= 0x32 && mouse_x < 0x140 && mouse_y < 0xc8)
        r = 3;
    if (mouse_x >= 0 && mouse_y >= 8 && map_pixel_size_x > mouse_x && map_pixel_size_y + 8 > mouse_y)
        r = 1;
    return r;
}

/* The editor's keys: the keypad and arrows move the cursor (7/9/1/3 diagonally, Home and
 * PgUp/PgDn/End the same), Backspace picks up the square (or the fill area with a feature
 * selected), Enter and Space paint the fill area, Esc, Ctrl-Q, Ctrl-X, Alt-Q and Alt-X ask
 * to exit as the Exit menu does. 1 when the key was taken. */
i16 parse_main_keys(void)
{
    register i16 r;
    i16 result;
    i16 dy;
    i16 dx;

    result = 1;
    dx = 0;
    dy = 0;
    switch (mykey) {
    default:
        result = 0;
        break;
    case 8:
        if (selected_right_mouse_change == 0)
            change_map(cursor_x, cursor_y, 1);
        else {
            make_undo();
            fill_map(cursor_x, cursor_y, 1);
        }
        break;
    case 0xd:
    case ' ':
        make_undo();
        fill_map(cursor_x, cursor_y, 0);
        break;
    case 0x1b:
    case 0x11:
    case 0x18:
    case 0x110:
    case 0x12d:
        if (map_changes == 0)
            game.game_in_progress = 0;
        else {
            msc_strcpy(popup_subst_string[0], map_name);
            msc_strupr(popup_subst_string[0]);
            r = popup_box("MAPEDIT", "EXIT", 0);
            if (r == 2) {
                if (write_map_file())
                    popup_box("MAPEDIT", "ERROR", 0);
                else
                    game.game_in_progress = 0;
            } else if (r == 1)
                game.game_in_progress = 0;
        }
        break;
    case '1':
    case 0x14f:
        dx = -1;
        dy = 1;
        break;
    case '2':
    case 0x150:
        dy = 1;
        break;
    case '3':
    case 0x151:
        dy = dx = 1;
        break;
    case '4':
    case 0x14b:
        dx = -1;
        break;
    case '6':
    case 0x14d:
        dx = 1;
        break;
    case '7':
    case 0x147:
        dy = dx = -1;
        break;
    case '9':
    case 0x149:
        dx = 1;
        dy = -1;
        break;
    case '8':
    case 0x148:
        dy = -1;
        break;
    }
    if (dx || dy) {
        move_cursor(dx, dy);
        result = 1;
    }
    return result;
}

/* parse_viewing_keys -- 0000:1f7e, 16 bytes */
/* Enter in viewing mode. */
i16 parse_viewing_keys(void)
{
    i16 r;

    r = 1;
    switch (mykey) {
    case 0xd:
        break;
    default:
        r = 0;
        break;
    }
    return r;
}

/* A mouse stroke over the map: the cursor follows the mouse; a click centres the view, a
 * shifted click paints (left: the square, or the fill area for a feature; right: pick up). */
i16 parse_area_map(void)
{
    i16 y;
    i16 x;
    i16 shift;

    y = 0;
    if (mouse_stroke_type != mouse_current_area)
        return 0;
    shift = shift_key();
    if (mouse_start_stroke)
        make_undo();
    if (mouse_any_stroke) {
        y = (mouse_y - 8) / (0x10 >> map_scale) - map_tile_inset_y + map_corner_y;
        x = mouse_x / (0x10 >> map_scale) - map_tile_inset_x + map_corner_x;
        if (on_map(x, y) == 0)
            return 0;
        if (cursor_x != x || cursor_y != y) {
            if (cursor_on)
                cursor_update();
            cursor_x = x;
            cursor_y = y;
        }
    }
    if (mouse_any_stroke) {
        if (shift == 0) {
            if (mouse_stop_stroke || mouse_button)
                set_center(x, y, 1);
        } else if (mouse_button && selected_right_mouse_change == 0)
            change_map(x, y, 1);
        else
            fill_map(x, y, mouse_button);
        if (mouse_stop_stroke && cursor_on == 0)
            cursor_update();
    }
    return 0;
}

/* A click on the minimap (at 0xfc, 9) centres the view there.
 * The sum is the same either way; a relink would swap the two operands' addresses.
 */
void parse_area_mini(void)
{
    register i16 x;
    register i16 y;

    if (mouse_stop_stroke) {
        y = mini_y + mouse_y - 9;
        x = minimax(mini_x + mouse_x - 0xfc, 1, map_file.w - 2);
        y = minimax(y, 1, map_file.h - 2);
        if (map_center_x != x || map_center_y != y)
            set_center(x, y, 1);
    }
}

/* parse_mouse_stroke -- 0000:20e0, 68 bytes */
/* Dispatches a mouse stroke to the area it started in: 1 the map, 2 the minimap, 3 the
 * info window (a click opens the selection screen). */
i16 parse_mouse_stroke(void)
{
    register i16 r;

    r = 0;
    mouse_current_area = mouse_area();
    if (mouse_start_stroke)
        mouse_stroke_type = mouse_current_area;
    switch (mouse_stroke_type) {
    case 1:
        r = parse_area_map();
        break;
    case 2:
        parse_area_mini();
        break;
    case 3:
        if (mouse_stop_stroke)
            selection_screen();
        break;
    }
    return r;
}

/* One pass of the editor's event loop: blinks the cursor every 20 ticks, runs the menu bar,
 * the keys (menu hotkeys, then the editor's, then viewing mode's) and the mouse, and the
 * chosen menu command. Returns when something was done. */
void human_interface_loop(void)
{
    register i16 done;
    register i16 any;

    cursor_clock = timer_read() + 0x14;
    done = 0;
    cursor_on = 0;
    cursor_update();
    mouse_init_cycle();
    do {
        game_clock = timer_read();
        mouse_begin_cycle(0);
        any = keys_any();
        if (any || menu_bar_mouse_parse(menu_bar, 0)) {
            if (cursor_on)
                cursor_clock = game_clock;
            else
                cursor_clock = game_clock + 0x14;
        }
        if (cursor_clock <= game_clock) {
            cursor_update();
            cursor_clock = game_clock + 0x14;
        }
        if (any) {
            mykey = keys_get();
            if (mykey < 0xff && islower(mykey))
                mykey -= 0x20;
            key_parsed = 0;
            key_parsed = menu_bar_key_parse(menu_bar, mykey);
            if (key_parsed == 0)
                key_parsed = menu_key_parse(menu_bar, mykey);
            if (key_parsed == 0)
                key_parsed = parse_main_keys();
            if (key_parsed == 0)
                key_parsed = parse_viewing_keys();
            cursor_clock = timer_read() + 0x14;
            done = 1;
        }
        if (any == 0 && menu_bar->f0 == 0) {
            menu_bar_mouse_parse(menu_bar, 1);
            if (menu_bar->f0 == 0)
                done |= parse_mouse_stroke();
        }
        if (menu_bar->f0 != 0) {
            done = 1;
            execute_menu_event(menu_bar->f0);
        }
        mouse_end_cycle(0, game.game_in_progress);
    } while (!done);
}

/* human_turn -- 0000:229c, 18 bytes */
void human_turn(void)
{
    draw_map_2();
    do
        human_interface_loop();
    while (game.game_in_progress);
}

void turn_control_loop(void)
{
    construct_mapedit_menu();
    menu_draw_bar(menu_bar, 0, 1);
    generate_mini();
    draw_map_2();
    do {
        info_window_draw();
        human_turn();
    } while (game.game_in_progress);
}
