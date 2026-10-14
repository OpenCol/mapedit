/* mapedit.c -- the editor: startup, the command line, the menu bar's commands, the file menu. */
/*
 * editor_main reads the command line and calls viceroy_game, which sets up the screen, the
 * clock, the mouse, the palette, the fonts and the art, gets a map ready (start_new_game) and
 * runs the editor (turn_control_loop, input.c) until it is left, then puts the screen back in
 * text mode. A failure on the way sets exit_value, which editor_main prints.
 *
 * The menu bar comes from MAPMENU.TXT, one section per title (game, view, cup, help), each
 * listing the title and then its items in order; construct_mapedit_menu gives every item its
 * command id and execute_menu_event carries the commands out. The popups' texts come from
 * MAPEDIT.TXT by section name (SAVE, EXIT, HELP1 ...).
 *
 * mapfile.c reads and writes the map files (.MP); this file only chooses them: map_name is
 * the current one, and file_menu offers the ones in the game folder.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "editstate.h"
#include "assets.h"
#include "edit.h"
#include "error.h"
#include "game.h"
#include "infopanel.h"
#include "input.h"
#include "mapview.h"
#include "theme.h"
#include "tilepicker.h"
#include "buffer.h"
#include "fileio.h"
#include "font.h"
#include "hspot.h"
#include "keys.h"
#include "loader.h"
#include "machine.h"
#include "map.h"
#include "mapedit.h"
#include "mcga.h"
#include "mem.h"
#include "menu.h"
#include "mouse.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "screen.h"
#include "sprite.h"
#include "text.h"
#include "timer.h"
#include "units.h"
#include "video.h"
#include "records.h"

/* Settings of the game's engine that the editor keeps but does not use: editor_main clears
 * mads_mode and file_set, and nothing reads either. */
i16 mads_mode = 0;
i16 file_set = 0;

/* Builds the menu bar from MAPMENU.TXT: the titles Editor (bar id 1), View (2), Map (6) and
 * Help (7, placed at the right), their items in the file's order with the command ids
 * execute_menu_event takes, and "" items as separators. Sets menu_bar. Returns 0, or 1 when
 * a section of MAPMENU.TXT is missing (the bar is then left half built). */
i16 construct_mapedit_menu(void)
{
    i16 result;

    result = 1;
    menu_bar = menu_create(0x800, menu_font);
    if (menu_bar != 0) {
        if (text_open("mapmenu", "game"))
            goto out;
        menu_add_bar_item(menu_bar, text_get(), 1, 0);
        menu_add_item(menu_bar, 1, text_get(), 0x1a);
        menu_add_item(menu_bar, 1, text_get(), 0x13);
        menu_add_item(menu_bar, 1, text_get(), 0x1b);
        menu_add_item(menu_bar, 1, text_get(), 0x14);
        menu_add_item(menu_bar, 1, "", 0);
        menu_add_item(menu_bar, 1, text_get(), 0x1f);
        if (text_open("mapmenu", "view"))
            goto out;
        menu_add_bar_item(menu_bar, text_get(), 2, 0);
        menu_add_item(menu_bar, 2, text_get(), 0x24);
        menu_add_item(menu_bar, 2, text_get(), 0x25);
        menu_add_item(menu_bar, 2, "", 0);
        menu_add_item(menu_bar, 2, text_get(), 0x26);
        menu_add_item(menu_bar, 2, text_get(), 0x27);
        menu_add_item(menu_bar, 2, text_get(), 0x28);
        menu_add_item(menu_bar, 2, text_get(), 0x29);
        menu_add_item(menu_bar, 2, "", 0);
        menu_add_item(menu_bar, 2, text_get(), 0x2b);
        if (text_open("mapmenu", "cup"))
            goto out;
        menu_add_bar_item(menu_bar, text_get(), 6, 0);
        menu_add_item(menu_bar, 6, text_get(), 0x4b);
        menu_add_item(menu_bar, 6, text_get(), 0x4c);
        menu_add_item(menu_bar, 6, "", 0);
        menu_add_item(menu_bar, 6, text_get(), 0x4d);
        menu_add_item(menu_bar, 6, text_get(), 0x4a);
        menu_add_item(menu_bar, 6, "", 0);
        menu_add_item(menu_bar, 6, text_get(), 0x4e);
        if (text_open("mapmenu", "help"))
            goto out;
        menu_add_bar_item(menu_bar, text_get(), 7, 1);
        menu_add_item(menu_bar, 7, text_get(), 0x51);
        menu_add_item(menu_bar, 7, text_get(), 0x52);
        menu_add_item(menu_bar, 7, text_get(), 0x53);
        menu_add_item(menu_bar, 7, text_get(), 0x54);
        menu_add_item(menu_bar, 7, "", 0);
        menu_add_item(menu_bar, 7, text_get(), 0x5f);
        result = 0;
    }
out:
    return result;
}
/* A file picker: a popup (section key of the text file `file`) listing the files that match
 * the DOS wildcard pattern in the game folder, ten to a page, with "(More)" items to step to
 * the previous and the next page. Counts the files into num_names and the pages into
 * num_screens; the chosen name is copied into file_names and file_select points at it.
 * Returns the chosen file's index on its page (0-9), or a negative value when there are no
 * files or the popup is cancelled. */
i16 file_menu(char *file, char *key, char *pattern)
{
    i16 screen;
    i16 nfiles;
    i16 lastn;
    void *popup;
    i16 keep;
    i16 i;
    i16 j;
    i16 base;
    i16 choice;
    struct find_t info;

    choice = -1;
    keep = 1;
    popup = 0;
    screen = 0;
    num_names = 0;
    if (msc__dos_findfirst(pattern, 0, &info) == 0) {
        do
            num_names++;
        while (msc__dos_findnext(&info) == 0);
    }
    if (num_names == 0)
        goto out;
    num_screens = (num_names + 9) / 10;
    do {
        i = -1;
        nfiles = 0;
        base = screen * 10;
        lastn = screen * 10 + 9;
        if (msc__dos_findfirst(pattern, 0, &info) == 0) {
            do {
                i++;
                if (i >= base && i <= lastn)
                    msc_strcpy(file_names[nfiles++], info.name);
            } while (msc__dos_findnext(&info) == 0 && nfiles < 10);
        }
        if (nfiles == 0)
            keep = 0;
        else {
            popup = popup_start_box(file, key, 0);
            if (popup == 0)
                goto out;
            if (screen)
                popup_add_item(popup, "(More)", 0x62);
            for (j = 0; j < nfiles; j++)
                popup_add_item(popup, file_names[j], j + 1);
            if (screen < num_screens - 1)
                popup_add_item(popup, "(More)", 0x63);
            keep = 0;
            choice = popup_exec(popup) - 1;
            if (choice == 0x61) {
                screen--;
                keep = 1;
            } else if (choice == 0x62) {
                screen++;
                keep = 1;
            } else if (choice >= 0)
                file_select = file_names[choice];
            mem_free(popup);
            popup = 0;
        }
    } while (keep);
out:
    if (popup)
        mem_free(popup);
    return choice;
}

/* The menu bar's record as memory_check reads it: the free count of the menu's arena, at
 * the position it had in 1994. */
typedef struct { char pad[0x4a]; i32 avail; } MenuArena;

/* Shows the free memory and the menu arena's free count in DEBUG.TXT's MEMORY popup.
 * Reached only through command 0x6a, which no menu item carries.
 * Port: MenuArena describes the 1994 layout of the Menu record. The port's Menu has wider
 * pointers, so on a 64-bit build `avail` falls on other bytes (part of the arena's base
 * pointer) and the number shown is meaningless. */
void memory_check(void)
{
    popup_subst_number[0] = mem_get_avail();
    popup_subst_number[1] = ((MenuArena *)menu_bar)->avail;
    popup_box("DEBUG", "MEMORY", 0);
}

/* Carries out the menu command with item id `id`:
 *   Editor  0x1a Save (after the SAVE popup's first choice), 0x13 Save As (asks for a name of
 *           up to 14 characters; the extension becomes .MP, and map_name changes even when
 *           the write then fails), 0x14 Create, 0x1b Load, 0x1f Exit;
 *   View    0x24 and 0x25 zoom in and out, 0x26-0x29 zoom levels 3 to 0, 0x2b centre the view
 *           on the cursor;
 *   Map     0x4a Find Continents, 0x4b Map Tile Select, 0x4c the brush size (cycles 1, 3, 5
 *           squares), 0x4d coastline protection on or off, 0x4e Undo;
 *   Help    0x51-0x54 the four help pages, 0x5f About.
 * 0x21 (view mode) and 0x6a (memory check) are handled but no menu item carries them.
 *
 * Create and Load ask first when the map has unsaved changes and go ahead on the popup's
 * first choice. Create then re-centres the cursor and the view and redraws the screen even
 * when no map was created. Load picks a file (MAPTOLOAD), loads it, converts its old forest
 * codes (forest_fix) and re-centres; a failed load shows ERROR. Exit leaves at once when the
 * map is unchanged, else after the EXIT popup (its second choice saves first, its first
 * leaves without saving). Undo restores the snapshot, if one was taken, and redraws; it does
 * nothing when there is no undo buffer. A failed save shows ERROR and keeps map_changes. */
void execute_menu_event(i16 id)
{
    register i16 r;

    switch (id) {
    case 0x1a:
        msc_strcpy(popup_subst_string[0], map_name);
        msc_strupr(popup_subst_string[0]);
        if (popup_box("MAPEDIT", "SAVE", 0) != 1)
            goto out;
        if (write_map_file()) {
            popup_box("MAPEDIT", "ERROR", 0);
            goto out;
        }
        map_changes = 0;
        goto out;
    case 0x13:
        msc_strcpy(popup_subst_string[0], map_name);
        msc_strupr(popup_subst_string[0]);
        if (popup_ask("MAPEDIT", "SAVEAS", popup_subst_string[0], 0xe))
            goto out;
        msc_strcpy(map_name, popup_text_buffer);
        fileio_new_ext(map_name, map_name, "MP");
        if (write_map_file())
            popup_box("MAPEDIT", "ERROR", 0);
        else
            map_changes = 0;
        goto out;
    case 0x14:
        msc_strcpy(popup_subst_string[0], map_name);
        msc_strupr(popup_subst_string[0]);
        r = !map_changes;
        if (!r)
            r = popup_box("MAPEDIT", "CREATENOW", 0) == 1 ? 1 : 0;
        if (r)
            create_me();
        cursor_x = map_center_x = map_file.w >> 1;
        cursor_y = map_center_y = map_file.h >> 1;
        new_mini();
        main_screen_refresh();
        goto out;
    case 0x1b:
        msc_strcpy(popup_subst_string[0], map_name);
        msc_strupr(popup_subst_string[0]);
        r = !map_changes;
        if (!r)
            r = popup_box("MAPEDIT", "LOAD", 0) == 1 ? 1 : 0;
        if (!r)
            goto out;
        if (file_menu("MAPEDIT", "MAPTOLOAD", "*.MP") < 0)
            goto out;
        msc_strcpy(map_name, file_select);
        if (load_map_file()) {
            popup_box("MAPEDIT", "ERROR", 0);
            goto out;
        }
        forest_fix();
        cursor_x = map_center_x = map_file.w >> 1;
        cursor_y = map_center_y = map_file.h >> 1;
        new_mini();
        main_screen_refresh();
        goto out;
    case 0x1f:
        msc_strcpy(popup_subst_string[0], map_name);
        msc_strupr(popup_subst_string[0]);
        if (map_changes == 0) {
            game.game_in_progress = 0;
            goto out;
        }
        r = popup_box("MAPEDIT", "EXIT", 0);
        if (r == 2) {
            if (write_map_file()) {
                popup_box("MAPEDIT", "ERROR", 0);
                goto out;
            }
            game.game_in_progress = 0;
        } else if (r == 1)
            game.game_in_progress = 0;
        goto out;
    case 0x21:
        set_view_mode();
        goto out;
    case 0x24:
        set_zoom_level(map_scale - 1);
        break;
    case 0x25:
        set_zoom_level(map_scale + 1);
        break;
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
        set_zoom_level(0x29 - id);
        break;
    case 0x2b:
        set_center(cursor_x, cursor_y, 1);
        break;
    case 0x4a:
        continent_check();
        break;
    case 0x4b:
        selection_screen();
        break;
    case 0x4c:
        fill_radius = (fill_radius + 1) % 3;
        info_window_draw();
        break;
    case 0x4d:
        coastline_protect = !coastline_protect;
        info_window_draw();
        break;
    case 0x4e:
        if (undo_available) {
            if (undo_active)
                perform_undo();
            main_screen_refresh();
        }
        break;
    case 0x51:
        popup_box("MAPEDIT", "HELP1", 0);
        break;
    case 0x52:
        popup_box("MAPEDIT", "HELP2", 0);
        break;
    case 0x53:
        popup_box("MAPEDIT", "HELP3", 0);
        break;
    case 0x54:
        popup_box("MAPEDIT", "HELP4", 0);
        break;
    case 0x5f:
        popup_box("MAPEDIT", "ABOUT", 0);
        break;
    case 0x6a:
        memory_check();
        break;
    }
out:;
}

#include "msc.h"

/* Unused here: the same layout as map.h's TerrainInfo. */
typedef struct { i16 name; char f2; char f3; char f4; char pad[2]; char v[9]; } TerrainRow;

/* Gets a map ready to edit. Unless the command line named one (-m) or asked for a new one
 * (-c), the MAPTOEDIT file picker chooses it; cancelling there creates a new map. The planes
 * are allocated at their full size: map_force is set, so map_startup does not read the
 * file's header (the test of (u16)map_force < 0 is never true). Then the map is loaded, its
 * old forest codes converted (forest_fix), or created (create_me), and the cursor and the
 * view go to its centre. Returns 0 when ready, 1 to quit; a failed load leaves map_error in
 * exit_value. */
i16 start_new_game(void)
{
    register i16 result;

    result = 1;
    if (map_selected == 0 && create_me_now == 0) {
        if (file_menu("MAPEDIT", "MAPTOEDIT", "*.MP") < 0)
            create_me_now = result;
        else
            msc_strcpy(map_name, file_select);
    }
    map_file.w = 0x3a;
    map_file.h = 0x48;
    map_force = result;
    if ((u16)map_force < 0)
        result = map_force;
    if (map_startup(result) == 0) {
        if (create_me_now != 0) {
            if (create_me())
                return result;
        } else {
            if (load_map_file()) {
                exit_value = map_error;
                return result;
            }
            forest_fix();
        }
        cursor_x = map_center_x = map_file.w >> 1;
        cursor_y = map_center_y = map_file.h >> 1;
        result = 0;
    }
    return result;
}
/* The editor from start to finish: the string store, the clock, mode 13h, the mouse, the
 * palette (VICEROY.PAL, kept past the visible screen in video memory, as in 1994), the work
 * and original screens, the fonts, the terrain tiles, the cursor, terrain-icon and icon
 * sheets, the backdrop tile (WOODTILE) that windows and menus are filled with, the terrain
 * table and colours (NAMES.TXT), the minimap colours and the undo buffer (12,000 bytes; Undo
 * is off without it). Then the map (start_new_game) and the editor (turn_control_loop).
 * Whatever happened, it hides the mouse, resets it for text mode and returns to video mode
 * 3, clearing the screen unless it was already in mode 3.
 *
 * exit_value says what could not be loaded: 0x13 the palette, 0x14 a screen buffer, 0x15
 * FONTINTR, 0x16 FONTTINY, 0x17 CURSOR, 0x18 PHYS0, 0x19 ICONS, 0x1a the backdrop buffer,
 * 0x1b WOODTILE; load_terrain_tiles sets its own (0x321, 0x322). */
void viceroy_game(void)
{
    void *wood;
    i16 mode;

    strings_init(0x7d0L);
    timer_install();
    screen_dominant_mode(0x13);
    video_init(0x13, 1);
    mouse_init(1, 0x13);
    mouse_show();
    mcga_compute_retrace_parameters();
    if (pal_load_2("viceroy.pal", (char *)(msc_vga + 0xfc00))) {
        exit_value = 0x13;
        goto out;
    }
    buffer_init(&scr_work, 0x140, 0xc8);
    if (scr_work.bits == 0) {
        exit_value = 0x14;
        goto out;
    }
    buffer_init(&scr_orig, 0x140, 0xc8);
    if (scr_orig.bits == 0) {
        exit_value = 0x14;
        goto out;
    }
    mouse_video_init();
    buffer_fill(0, scr_work);
    video_update(&scr_work, 0, 0, 0, 0, 0x140, 0xc8);
    font_inter = font_load("fontintr");
    if (font_inter == 0) {
        exit_value = 0x15;
        goto out;
    }
    menu_font = font_load("fonttiny");
    if (menu_font == 0) {
        exit_value = 0x16;
        goto out;
    }
    popup_init(font_inter, 0x800);
    mcga_setpal((char *)(msc_vga + 0xfc00));
    if (load_terrain_tiles())
        goto out;
    cursor = sprite_series_load("cursor", 0x4000);
    if (cursor == 0) {
        exit_value = 0x17;
        goto out;
    }
    tiles = sprite_series_load("phys0", 0x4000);
    tiles2 = 0;
    if (tiles == 0) {
        exit_value = 0x18;
        goto out;
    }
    tiles2 = tiles;
    icons = sprite_series_load("icons", 0x4000);
    if (icons == 0) {
        exit_value = 0x19;
        goto out;
    }
    buffer_init(&scr_back, 0x20, 0x18);
    if (scr_back.bits == 0) {
        exit_value = 0x1a;
        goto out;
    }
    wood = sprite_series_load("woodtile", 0x4000);
    if (wood == 0) {
        exit_value = 0x1b;
        goto out;
    }
    sprite_draw(1, wood, &scr_back, 0, 0);
    mem_free(wood);
    main_tile = menu_tile = popup_tile = &scr_back;
    get_tile_colors();
    load_data();
    theme_load();
    menu_select_color_0 = menu_bar_select_color_0 = select_color;
    menu_font_main_color = menu_bar_font_main_color = basic_color;
    menu_font_grey_color = menu_bar_font_grey_color = grey_color;
    menu_font_hilite_color = menu_bar_font_hilite_color = hilite_color;
    initialize_map_memory();
    map_undo_memory = mem_get(0x2ee0L);
    if (map_undo_memory != 0)
        undo_available = 1;
    if (start_new_game() == 0) {
        mcga_setpal((char *)(msc_vga + 0xfc00));
        game.game_in_progress = 1;
        turn_control_loop();
    }
out:
    mouse_hide();
    mode = video_mode;
    mouse_init(0, 3);
    video_init(3, mode != 3 ? 1 : 0);
    timer_remove();
}
/* Prints the program's name and copyright. */
void show_logo(void)
{
    msc_printf("Colonization Map Editor\n");
    msc_printf("Copyright (C) 1994 by Microprose Software\n\n");
}
/* Prints the name and the command-line options (for a "?" argument). */
void show_flags(void)
{
    show_logo();
    msc_printf("Options:\n");
    msc_printf("  mapedit           = Starts editor normally\n");
    msc_printf("  mapedit -c        = Force creation of new map\n");
    msc_printf("  mapedit -m:file   = Edits specified file\n");
}
/* Advances *p past the next c in the string; returns 1 when c was found, else 0. When the
 * string ends, *p is left on its last character rather than on the terminator, so the
 * caller's own step lands on the terminator. scan_past(p, 0) moves to the end. */
i16 scan_past(char **p, char c)
{
    i16 r;

    r = 0;
    if (**p != 0) {
        while (**p != c) {
            (*p)++;
            if (**p == 0)
                break;
        }
    }
    if (c != 0 && **p == c) {
        (*p)++;
        r = 1;
    }
    if (**p == 0)
        (*p)--;
    return r;
}
/* One command-line flag letter at *p, either case: C creates a new map; M:file names the map
 * to edit (sets map_name, and skips *p to the end of the argument). An M without ":file"
 * still marks the map as chosen, leaving map_name as it was. Other letters are ignored. */
void flag_parse(char **p)
{
    i16 c;

    c = **p;
    switch (islower(c) ? c - 0x20 : c) {
    case 'C':
        create_me_now = 1;
        break;
    case 'M':
        if (scan_past(p, ':')) {
            msc_strcpy(map_name, *p);
            scan_past(p, 0);
        }
        map_selected = 1;
        break;
    }
}
/* The program's main, called from port/main.c. Each argument starting with - or / is a run
 * of flags, every letter of it passed to flag_parse (so flags may be run together: -cm:FILE);
 * an argument starting with ? prints the options and quits without editing (-? does not).
 * Other arguments are ignored. Enables the FAB codec for the art, runs the editor, and
 * prints "Exit value: n" when it failed. */
void editor_main(i16 argc, char **argv)
{
    register i16 i;
    char *s;

    pack_enable_pfab();
    mads_mode = 0;
    file_set = 0;
    for (i = 1; i < argc; i++) {
        if (msc_strchr("-/", argv[i][0])) {
            s = argv[i];
            while (*s != 0) {
                flag_parse(&s);
                s++;
            }
        } else if (argv[i][0] == '?') {
            show_flags();
            goto done;
        }
    }
    viceroy_game();
done:
    if (exit_value)
        msc_printf("Exit value: %d\n", exit_value);
}

char *file_select;                        /* the name file_menu chose (into file_names) */
char file_names[10][13];                  /* file_menu's page: up to ten 8.3 names */

