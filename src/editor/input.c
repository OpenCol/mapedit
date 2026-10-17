/* input.c -- the editor's input: the main loop, the keys, mouse strokes on the map and minimap. */
/*
 * turn_control_loop runs the editor once the map is loaded: it builds the menu bar and draws
 * the screen, then human_interface_loop runs until game.game_in_progress is cleared (Exit).
 * Each pass of that loop reads the 60 Hz clock, blinks the map cursor, and hands a key first
 * to the menu bar (Alt+letter, then the items' hotkeys), then to the editor's own keys; a
 * mouse stroke goes to the menu bar, then to the screen area it started in (the map, the
 * minimap or the info window). A menu choice lands in menu_bar->f0 and runs through
 * execute_menu_event (mapedit.c).
 *
 * The screen is 320 x 200: the menu bar along the top 8 rows, the map view below it from the
 * left edge (map_pixel_size_x x map_pixel_size_y), the minimap at the top right and the info
 * window beneath it.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
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
#include "options.h"
#include "mapview.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "tilepicker.h"
#include "timer.h"

i32 game_clock;                          /* the 60 Hz clock, read once per loop pass */

i16 key_parsed;                           /* nonzero when some handler took the last key */

i16 mykey;                                /* the key being handled (keys_get's code) */

i16 mouse_stroke_type;                    /* the screen area the current stroke began in */

i16 mouse_current_area;                   /* the screen area the mouse is in now */

i32 cursor_clock;                        /* when the map cursor next blinks */

/* Waits for a key or the end of a mouse stroke; returns the key, or 0 for the mouse. */
i16 event_wait(void)
{
    i16 running;
    i16 key;

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

/* Nonzero while either Shift key is held (the BIOS keyboard flags' two low bits). */
i16 shift_key(void)
{
    return *(u8 *)(msc_bda + 0x0017) & 3;
}

/* Which screen area the mouse is in: 1 the map view ((0, 8) and map_pixel_size_x by
 * map_pixel_size_y), 2 the minimap ((0xfc, 9) to (0x134, 0x30)), 3 the info window ((0xf1,
 * 0x32) to the bottom-right corner), 0 anywhere else. The map is tested last and wins. */
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

/* The editor's own keys, in mykey:
 *   - the arrows and the digits 1-9 (the keypad with Num Lock) move the cursor; Home, PgUp,
 *     End and PgDn and 7, 9, 1, 3 move it diagonally;
 *   - Backspace picks up the cursor's square, or with a feature selected erases the feature
 *     over the brush (taking an undo snapshot first);
 *   - Enter and Space paint the brush at the cursor (taking an undo snapshot first);
 *   - Esc, Ctrl-Q, Ctrl-X, Alt-Q and Alt-X leave as the Exit command does: at once when the
 *     map is unchanged, else after the EXIT popup (its second choice saves first, its first
 *     leaves without saving, anything else stays);
 *   - port, with the undo history (options.h): Ctrl-Z is Undo and Ctrl-Y Redo.
 * Returns 1 when the key was one of these, else 0. */
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
                r = checked_write_map();
                if (r > 0)
                    popup_box("MAPEDIT", "ERROR", 0);
                else if (r == 0)
                    game.game_in_progress = 0;
            } else if (r == 1)
                game.game_in_progress = 0;
        }
        break;
    case 0x1a:
    case 0x19:
        if (editor_options.undo_levels)
            execute_menu_event(mykey == 0x1a ? 0x4e : 0x72);
        else
            result = 0;
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

/* The viewing mode's keys: takes Enter (already taken by parse_main_keys) and does nothing
 * with it. Returns 1 for Enter, else 0. */
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

/* A mouse stroke over the map view, while it stays there. The cursor follows the mouse.
 * Without Shift, releasing the button (or holding the right one) centres the view on the
 * square. With Shift, the left button paints the brush; the right button picks up the square
 * with a terrain selected, or erases the feature over the brush with a feature selected.
 *
 * Every stroke takes an undo snapshot when it starts, even one that only moves the view, so
 * Undo reaches back only to the start of the last stroke. Always returns 0. */
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

/* Releasing the button over the minimap centres the view on that square, kept inside the
 * map's border. Does nothing when the view is already centred there. */
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

/* Sends the mouse to the handler of the area the current stroke started in: the map view,
 * the minimap, or the info window (where releasing the button opens Map Tile Select).
 * Returns what the map's handler returns, else 0. */
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

/* The editor's event loop, run until a key or a menu command has been handled. Each pass,
 * paced to the 60 Hz clock:
 *   - toggles the map cursor every 20 ticks (a third of a second). A key, or a press on the
 *     menu bar, first makes the next toggle due at once if the cursor is showing, so the
 *     cursor is hidden while the key or the menu is handled;
 *   - a key (lower-case letters made upper case) goes to the menu bar's Alt+letter titles, the
 *     menu items' hotkeys, parse_main_keys and parse_viewing_keys, until one takes it;
 *   - without a key and with no menu chosen, the mouse goes to the menu bar and then, if that
 *     did not choose anything, to parse_mouse_stroke;
 *   - a menu choice (menu_bar->f0) runs through execute_menu_event.
 * The cursor is drawn when the loop starts. */
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

/* Redraws the map, then runs the event loop until the editor is left. */
void human_turn(void)
{
    draw_map_2();
    do
        human_interface_loop();
    while (game.game_in_progress);
}

/* Runs the editor on the loaded map: builds and draws the menu bar, places the minimap,
 * draws the map and the info window, and returns when the editor is left (human_turn only
 * returns then, so the loop runs once). */
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
