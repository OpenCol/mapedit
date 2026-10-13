/* From mapedit-decomp src/editor/mapedit (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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
/* MODULE mapedit: MAPEDIT.EXE segment 0000 (mapedit.obj), file 0x1600-0x3f7c, 64 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn reset_sprite_memory      0x01600
 * fn load_main_sprites        0x01602
 * fn check_music              0x01604
 * fn build_check              0x01606
 * fn piece_draw               0x01608
 * fn sprite_load              0x0160a
 * fn danger                   0x0160c
 * fn perform_save             0x0160e
 * fn piss_factor              0x01610
 * fn pissed                   0x01612
 * fn say_country              0x01614
 * fn cycle_save               0x01616
 * fn popups_normal            0x01618
 * fn event_wait               0x01660
 * fn forest_fix               0x016b6
 * fn shift_key                0x01786
 * fn construct_mapedit_menu   0x01796
 * fn file_menu                0x01b6a
 * fn make_undo                0x01d50
 * fn perform_undo             0x01d7e
 * fn info_window_clear        0x01da6
 * fn info_window_update       0x01e2c
 * fn info_terrain             0x01e42
 * fn info_terrain_2           0x01ee2
 * fn info_window_draw         0x01f4e  (914 bytes)
 * fn main_screen_refresh      0x022e0  (74 bytes)  near `ovl22_6b6c4` blank_screen (0.40)
 * fn add_item                 0x0232a  (116 bytes)
 * fn add_item_2               0x0239e  (118 bytes)
 * fn selection_screen_display   0x02414  (696 bytes)
 * fn parse_spot               0x026cc  (290 bytes)
 * fn selection_mouse          0x027ee  (56 bytes)
 * fn selection_screen         0x02826  (234 bytes)
 * fn cursor_update            0x02910  (244 bytes)
 * fn set_center               0x02a04  (90 bytes)
 * fn possibly_center          0x02a5e  (182 bytes)
 * fn move_cursor              0x02b14  (102 bytes)
 * fn set_view_mode            0x02b7a  (8 bytes)
 * fn set_zoom_level           0x02b82  (74 bytes)
 * fn memory_check             0x02bcc  (48 bytes)
 * fn create_me                0x02bfc  (116 bytes)
 * fn continent_check          0x02c70  (368 bytes)
 * fn execute_menu_event       0x02de0  (932 bytes)
 * fn mouse_area               0x03184  (92 bytes)
 * fn change_map               0x031e0  (328 bytes)
 * fn fill_map                 0x03328  (142 bytes)
 * fn parse_main_keys          0x033b6  (456 bytes)
 * fn parse_viewing_keys       0x0357e  (16 bytes)
 * fn parse_area_map           0x0358e  (244 bytes)
 * fn parse_area_mini          0x03682  (94 bytes)
 * fn parse_mouse_stroke       0x036e0  (68 bytes)
 * fn human_interface_loop     0x03724  (376 bytes)
 * fn human_turn               0x0389c  (18 bytes)
 * fn computer_turn            0x038ae  (2 bytes)
 * fn turn_control_loop        0x038b0  (52 bytes)
 * fn load_terrain             0x038e4  (82 bytes)  near `ovl26_73270` load_terrain (0.63)
 * fn load_data                0x03936  (324 bytes)
 * fn start_new_game           0x03a7a  (156 bytes)
 * fn viceroy_game             0x03b16  (712 bytes)
 * fn show_logo                0x03dde  (24 bytes)
 * fn show_flags               0x03df6  (50 bytes)
 * fn scan_past                0x03e28  (74 bytes)
 * fn flag_parse               0x03e72  (102 bytes)
 * fn main                     0x03ed8  (164 bytes)
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * THE MAP EDITOR'S MAIN MODULE. The first twelve functions are empty stubs standing in for
 * game functions the shared madsdev/game modules reference (popup.obj calls cycle_save,
 * reset_sprite_memory, sprite_load, pissed, piss_factor; the others are the game's hooks).
 * The module's own data is defined here: the editor state and colour bytes at DS:0x42-0x9b
 * (mads_mode .. border2, above shift_key, so they precede the first literal), myspot at
 * DS:0x1ce (after info_window_draw's literals), and the BSS communals from DS:0x49c6
 * (video_mode .. country_names) after the last function.
 */

/* popups_normal -- 0000:0018, 72 bytes */

/* event_wait -- 0000:0060, 86 bytes */

/* forest_fix -- 0000:00b6, 208 bytes */

/* shift_key -- 0000:0186, 16 bytes */
/* mapedit.obj's initialised data, defined here (the 42 land at DS:0x0042-0x009c in this
 * order, ahead of construct_mapedit_menu's "game", the module's first literal: MSC emits a
 * global after the literals of the function that follows its definition, and shift_key has
 * none). A global first declared further down keeps a pad there; 2 pads after the block bring
 * the 32 symbols it adds to 0 mod 17. */
i16 mads_mode = 0;                              /* DS:0x0042 */
i16 file_set = 0;                               /* DS:0x0044 */
i16 load_error = 0;                             /* DS:0x0060 */
i16 exit_value = 0;                             /* DS:0x0062 */
Menu *menu_bar = 0;                         /* DS:0x0078 */
void *game_menu = 0;                        /* DS:0x007c */

/* construct_mapedit_menu -- 0000:0196, 980 bytes */

/* The menu bar from MAPMENU.TXT: Editor (1), View (2), Map (6), Help (7). 1 on failure. */
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
/* file_menu -- 0000:056a, 486 bytes */

/* The file picker (VICEROY's choose_file): the files matching pattern, ten per page with
 * (More) items at 0x62/0x63; the choice's name is left in file_select. Returns the index
 * chosen, -1 for none. */
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
/* make_undo -- 0000:0750, 46 bytes */

/* info_window_clear -- 0000:07a6, 134 bytes */

/* info_terrain -- 0000:0842, 160 bytes */

/* info_window_draw -- 0000:094e, 914 bytes */

/* main_screen_refresh -- 0000:0ce0, 74 bytes */

/* selection_mouse -- 0000:11ee, 56 bytes */

/* cursor_update -- 0000:1310, 244 bytes */

/* set_center -- 0000:1404, 90 bytes */

/* possibly_center -- 0000:145e, 182 bytes */

/* set_zoom_level -- 0000:1582, 74 bytes */

/* memory_check -- 0000:15cc, 48 bytes */
typedef struct { char pad[0x4a]; i32 avail; } MenuArena;

/* The free memory and the menu arena's free count, in DEBUG.TXT's MEMORY popup. */
void memory_check(void)
{
    popup_subst_number[0] = mem_get_avail();
    popup_subst_number[1] = ((MenuArena *)menu_bar)->avail;
    popup_box("DEBUG", "MEMORY", 0);
}
/* create_me -- 0000:15fc, 116 bytes */

/* continent_check -- 0000:1670, 368 bytes */

/* execute_menu_event -- 0000:17e0, 932 bytes */

/* The menu bar's commands by item id (MAPMENU.TXT): Editor 0x13 Save As, 0x14 Create,
 * 0x1a Save, 0x1b Load, 0x1f Exit; View 0x21 view mode, 0x24/0x25 zoom in/out, 0x26-0x29
 * zoom levels, 0x2b centre; Map 0x4a continents, 0x4b tile select, 0x4c fill mode, 0x4d
 * coastline protect, 0x4e undo; Help 0x51-0x54, 0x5f about; 0x6a the memory check. */
/* Source order: Save (0x1a) before Save As (0x13): the literals say so, while MSC lays the
 * arms out in case-value order. */
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
/* mouse_area -- 0000:1b84, 92 bytes */

/* change_map -- 0000:1be0, 328 bytes */

/* fill_map -- 0000:1d28, 142 bytes */

/* parse_main_keys -- 0000:1db6, 456 bytes */
#include "msc.h"

/* parse_area_map -- 0000:1f8e, 244 bytes */

/* parse_area_mini -- 0000:2082, 94 bytes */

/* human_interface_loop -- 0000:2124, 376 bytes */

/* turn_control_loop -- 0000:22b0, 52 bytes */

/* load_terrain -- 0000:22e4, 82 bytes */
typedef struct { i16 name; char f2; char f3; char f4; char pad[2]; char v[9]; } TerrainRow;

/* start_new_game -- 0000:247a, 156 bytes */

/* Picks the map to edit (MAPTOEDIT, or -m:file; Esc creates one), allocates the map and
 * loads or creates it, cursor at the centre. 0 when ready, 1 to quit. */
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
/* viceroy_game -- 0000:2516, 712 bytes */

/* The editor's main: video mode 0x13, the mouse, the palette, the screen buffers, the fonts,
 * the tile and icon sheets, the backdrop tile (WOODTILE), the data and the undo buffer; then
 * the map and the editor loop; exit_value 0x13-0x1b names what failed to load. */
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
/* show_logo -- 0000:27de, 24 bytes */

void show_logo(void)
{
    msc_printf("Colonization Map Editor\n");
    msc_printf("Copyright (C) 1994 by Microprose Software\n\n");
}
/* show_flags -- 0000:27f6, 50 bytes */
void show_flags(void)
{
    show_logo();
    msc_printf("Options:\n");
    msc_printf("  mapedit           = Starts editor normally\n");
    msc_printf("  mapedit -c        = Force creation of new map\n");
    msc_printf("  mapedit -m:file   = Edits specified file\n");
}
/* scan_past -- 0000:2828, 74 bytes */
/* Advances *p to the next c (or the end); 1 when c was found and skipped. The pointer is
 * left on the last character rather than past it. */
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
/* flag_parse -- 0000:2872, 102 bytes */
/* A command-line flag: C creates a new map, M:file names the map to edit. */
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
/* main -- 0000:28d8, 164 bytes */

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

/* @@END@@ */

/* mapedit.obj's uninitialised data: communals, placed by the linker in BSS (from DS:0x4999).
 * Sizes are the distance to the next public; a global nothing in the editor reads keeps a
 * plain array of that size. A definition after the last function moves no symbol position. */
i16 video_mode;                           /* DS:0x49c6 */
char *file_select;                        /* DS:0x49ee */
char file_names[10][13];                  /* DS:0x64f0: 130 bytes */

