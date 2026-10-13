/* Imported from mapedit-decomp src/editor/mapedit by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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
 * The module's own data: the colour bytes and editor state at DS:0x42-0x9b (mads_mode ..
 * border2), myspot at DS:0x1ce, and the BSS globals from DS:0x49c6 (video_mode .. country_names).
 * All of it is declared extern here, so the file grades without defining the data.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 height; } DosFont;
typedef struct { i16 w, h; } MapSize;
/* reset_sprite_memory -- 0000:0000, 2 bytes */
void reset_sprite_memory(void)
{
}
/* load_main_sprites -- 0000:0002, 2 bytes */
void load_main_sprites(void)
{
}
/* check_music -- 0000:0004, 2 bytes */
void check_music(void)
{
}
/* build_check -- 0000:0006, 2 bytes */
void build_check(void)
{
}
/* piece_draw -- 0000:0008, 2 bytes */
void piece_draw(void)
{
}
/* sprite_load -- 0000:000a, 2 bytes */
void sprite_load(void)
{
}
/* danger -- 0000:000c, 2 bytes */
void danger(void)
{
}
/* perform_save -- 0000:000e, 2 bytes */
void perform_save(void)
{
}
/* piss_factor -- 0000:0010, 2 bytes */
void piss_factor(void)
{
}
/* pissed -- 0000:0012, 2 bytes */
void pissed(void)
{
}
/* say_country -- 0000:0014, 2 bytes */
void say_country(void)
{
}
/* cycle_save -- 0000:0016, 2 bytes */
void cycle_save(void)
{
}
/* popups_normal -- 0000:0018, 72 bytes */
extern u8 basic_color, hilite_color, grey_color, enhance_color, shadow_color;   /* DS:0x92.. */
extern u8 select_color, notice_color, border0, border1, border2;                /* DS:0x97.. */
extern i16 popup_select_color_0, popup_select_color_1;          /* DS:0x540, 0x542 */
extern i16 popup_border_color, popup_border_color_up, popup_border_color_down;   /* DS:0x544.. */
extern i16 popup_font_main_color, popup_font_grey_color, popup_font_hilite_color; /* DS:0x54a.. */
extern i16 popup_enhance_color, popup_shadow_color;             /* DS:0x550, 0x552 */
extern i16 popup_pal;                                           /* DS:0x564 */
extern DosPort *popup_tile;                                     /* DS:0x56c */
extern DosPort scr_back;                                        /* DS:0x49fa */

/* The popup colours for the editor's screens, from the editor's colour bytes. */
void popups_normal(void)
{
    popup_enhance_color = enhance_color;
    popup_shadow_color = shadow_color;
    popup_font_main_color = basic_color;
    popup_font_hilite_color = hilite_color;
    popup_font_grey_color = grey_color;
    popup_select_color_0 = popup_select_color_1 = select_color;
    popup_border_color = border0;
    popup_border_color_up = border1;
    popup_border_color_down = border2;
    popup_tile = &scr_back;
    popup_pal = 1;
}
/* event_wait -- 0000:0060, 86 bytes */
extern i32 timer_read(void);
extern void mouse_init_cycle(void);
extern void mouse_begin_cycle(i16 a);
extern i16 keys_any(void);
extern i16 keys_get(void);
extern void mouse_end_cycle(i16 a, i16 hold);
extern i16 mouse_stop_stroke;                                   /* DS:0x730 */

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
/* forest_fix -- 0000:00b6, 208 bytes */
extern MapSize map_file;                                        /* DS:0x4b12 */
extern DosPort scr_map;                                         /* DS:0x4e92 */
extern u8 buffer_get(i16 x, i16 y, DosPort port);
extern void buffer_put(i16 x, i16 y, i16 v, DosPort port);

/* Old map files: a hills/mountain bit (0x20) on a forest square keeps only its base terrain
 * (bits 0xe0 and the low 3), and a plain forest (0x10-0x17) becomes its unforested type.
 * The `type -= 0x10; type += 8;` pair is what gives the shipped `add word [type],-8`: a
 * single -8 in any spelling comes out as `sub ..,8`. */
void forest_fix(void)
{
    i16 type;
    i16 byte;
    i16 x;
    i16 y;

    for (x = 0; x < map_file.w; x++) {
        for (y = 0; y < map_file.h; y++) {
            byte = buffer_get(x, y, scr_map);
            type = (u8)(byte & 0x1f);
            if (type >= 0x18)
                continue;
            if (byte & 0x20) {
                byte = byte & 0xe0;
                type = type & 7;
                buffer_put(x, y, byte | type, scr_map);
                continue;
            }
            if (type < 0x10 || type >= 0x18)
                continue;
            type = buffer_get(x, y, scr_map);
            type -= 0x10;
            type += 8;
            buffer_put(x, y, type, scr_map);
        }
    }
}
/* shift_key -- 0000:0186, 16 bytes */
i16 shift_key(void)
{
    return *(u8 *)(msc_bda + 0x0017) & 3;
}
/* construct_mapedit_menu -- 0000:0196, 980 bytes */
typedef struct { i16 f0; } MenuHead;
extern void *menu_bar;                                      /* DS:0x78 */
extern DosFont *menu_font;                                  /* DS:0x80 */
extern void *menu_create(i16 size, DosFont *font);
extern i16 text_open(char *name, char *key);
extern char *text_get(void);
extern void *menu_add_bar_item(void *m, char *name, i16 id, i16 right);
extern void *menu_add_item(void *m, i16 sub_id, char *name, i16 id);

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
extern i16 num_names;                                           /* DS:0x521c */
extern i16 num_screens;                                         /* DS:0x4e78 */
extern char file_names[][13];                                   /* DS:0x64f0 */
extern char *file_select;                                       /* DS:0x49ee */
extern void * popup_start_box(char *txt, char *key, i16 sel);
extern void *popup_add_item(void *p, char *s, i16 key);
extern i16 popup_exec(void *p);
extern void mem_free(void *p);

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
extern i16 undo_available;                                      /* DS:0x50 */
extern i16 undo_active;                                         /* DS:0x52 */
extern void *map_undo_memory;                               /* DS:0x5a */
extern void *map;                                           /* DS:0x4a8 */

/* Copies the terrain plane (0x2ee0 bytes) into the undo buffer. Both have a dead
 * `result = -1` (the `mov ax,-1`) and 6 more bytes of unused frame. */
void make_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_available) {
        msc__fmemcpy(map_undo_memory, map, 0x2ee0);
        undo_active = 1;
    }
}
/* perform_undo -- 0000:077e, 40 bytes */
void perform_undo(void)
{
    i16 result;
    i32 size;
    i16 n;

    result = -1;
    if (undo_active)
        msc__fmemcpy(map, map_undo_memory, 0x2ee0);
}
/* info_window_clear -- 0000:07a6, 134 bytes */
extern DosPort *main_tile;                                      /* DS:0x90 */
extern DosPort scr_work;                                        /* DS:0x3af4 */
extern void buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);
extern void buffer_tile(DosPort dst, DosPort src, i16 x, i16 y, i16 w, i16 h, i16 sx, i16 sy);
extern void buffer_draw_box(i16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color);
extern void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

/* The info window at (0xf1, 0x32), 0x4f x 0x96: the backdrop tile or colour 0x22, framed. */
void info_window_clear(void)
{
    if (main_tile == 0)
        buffer_rect_fill(0xf1, 0x32, 0x4f, scr_work, 0x96, 0x22);
    else
        buffer_tile(scr_work, *main_tile, 0xf1, 0x32, 0x4f, 0x96, 0, 0);
    buffer_draw_box(0xf0, 0x31, 0x140, scr_work, 0xc8, 0);
}
/* info_window_update -- 0000:082c, 22 bytes */
void info_window_update(void)
{
    mouse_video_update(0xf1, 0x32, 0xf1, 0x32, 0x4f, 0x96);
}
/* info_terrain -- 0000:0842, 160 bytes */
typedef struct { i16 name; char f2; char f3; char f4; char pad[2]; char v[9]; } TerrainRec;   /* NAMES.TXT's terrain rows */
extern TerrainRec terrain[];                                    /* DS:0x4ee6 */
extern i16 terrain_names[];                                     /* DS:0x4b5a */
extern char *strings(i16 n);
extern void space_string(char *buf);
extern i16 write_string(char *s, i16 x, i16 y);

/* Writes "(terrain name)" at (*x, *y) and moves *y down a line. Forests (8-0x17) get the
 * word terrain_names[0] after the name. */
void info_terrain(i16 t, i16 *x, i16 *y)
{
    char buf[0x50];

    msc_strcpy(buf, "(");
    msc__fstrcat(buf, strings(terrain[t].name));
    if (t >= 8 && t < 0x18) {
        space_string(buf);
        msc__fstrcat(buf, strings(terrain_names[0]));
    }
    msc_strcat(buf, ")");
    write_string(buf, *x, *y);
    *y += menu_font->height + 1;
}
/* info_terrain_2 -- 0000:08e2, 108 bytes */
/* The same for a feature word (terrain_names[n]). */
void info_terrain_2(i16 n, i16 *x, i16 *y)
{
    char buf[0x50];

    msc_strcpy(buf, "(");
    msc__fstrcat(buf, strings(terrain_names[n]));
    msc_strcat(buf, ")");
    write_string(buf, *x, *y);
    *y += menu_font->height + 1;
}
/* info_window_draw -- 0000:094e, 914 bytes */
extern i16 cursor_x;                                            /* DS:0x4b52 */
extern i16 cursor_y;                                            /* DS:0x4b54 */
extern u8 map_get(i16 x, i16 y);
extern i16 terrain_type(i16 b);
extern void *tiles;                                         /* DS:0x4c0 */
extern void *terrain_1;                                     /* DS:0x4b8 */
extern void sprite_draw(i16 icon, void *sheet, DosPort *port, i16 x, i16 y);
extern void tile_draw(void *tiles, i16 tile, DosPort *port, i16 x, i16 y);
extern char selected_terrain;                                   /* DS:0x56 */
extern u8 selected_physical_and_mask;                /* DS:0x57 */
extern u8 selected_physical_or_mask;                 /* DS:0x58 */
extern char selected_right_mouse_change;                        /* DS:0x59 */
extern i16 fill_radius;                                         /* DS:0x4a */
extern i16 coastline_protect;                                   /* DS:0x4e */

/* 2 pads: the operand order of the `selected_terrain != terr` byte compare (mod 17) */
extern i16 pad_iw0;
extern i16 pad_iw1;
/* The info window: map size, cursor, the terrain under the cursor (with its hills/mountains),
 * the selected terrain or feature with its tile and the shift-click legend, the fill radius
 * and the coastline protect state. */
void info_window_draw(void)
{
    char tmp[0x50];
    i16 type;
    i16 x;
    i16 y;
    i16 flag;
    i16 terr;
    i16 under;

    info_window_clear();
    y = 0x33;
    x = 0xf2;
    flag = 1;
    msc_sprintf(tmp, "Size: (%d, %d)", map_file.w, map_file.h);
    write_string(tmp, x, y);
    y += menu_font->height + 1;
    msc_sprintf(tmp, "Curs: (%d, %d)", cursor_x, cursor_y);
    write_string(tmp, x, y);
    y += menu_font->height + 6;
    write_string("Terrain at cursor:", x, y);
    y += menu_font->height + 1;
    under = map_get(cursor_x, cursor_y);
    type = (u8)(under & 0x1f);
    info_terrain(terrain_type(under), &x, &y);
    if (under & 0x40) {
        if (under & 0x80)
            info_terrain_2(2, &x, &y);
        else
            info_terrain_2(3, &x, &y);
    }
    y += 0xa;
    write_string("Selected:", x, y);
    y += menu_font->height + 1;
    if (selected_right_mouse_change) {
        if (selected_physical_or_mask & 0x40) {
            if (selected_physical_or_mask & 0x80)
                sprite_draw(4, tiles, &scr_work, x, y);
            else
                sprite_draw(0x14, tiles, &scr_work, x, y);
        } else if (selected_physical_or_mask & 0x20) {
            if (selected_physical_or_mask & 0x80)
                sprite_draw(0x24, tiles, &scr_work, x, y);
            else
                sprite_draw(0x34, tiles, &scr_work, x, y);
        }
    } else {
        terr = (selected_terrain <= 7 || selected_terrain == 0x11 || selected_terrain >= 0x18)
               ? selected_terrain : selected_terrain & 7;
        tile_draw(terrain_1, terr, &scr_work, x, y);
        if (selected_terrain != (char)terr)
            sprite_draw(0x41, tiles, &scr_work, x, y);
    }
    y += 0x14;
    if (flag == 0)
        info_terrain_2(4, &x, &y);
    else if (selected_physical_or_mask & 0x20) {
        if (selected_physical_or_mask & 0x80)
            info_terrain(0x1b, &x, &y);
        else
            info_terrain(0x1c, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Add", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Remove", x, y);
    } else if (selected_physical_or_mask & 0x40) {
        if (selected_physical_or_mask & 0x80)
            info_terrain_2(2, &x, &y);
        else
            info_terrain_2(3, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Add", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Remove", x, y);
    } else {
        info_terrain(selected_terrain, &x, &y);
        y += menu_font->height + 5;
        write_string("Shift-Left:  Paint", x, y);
        y += menu_font->height + 1;
        write_string("Shift-Right: Pick up", x, y);
    }
    y += 0x14;
    msc_sprintf(tmp, "Fill radius: %d", fill_radius);
    write_string(tmp, x, y);
    y += menu_font->height + 1;
    if (coastline_protect)
        write_string("Coast Protect: ON", x, y);
    else
        write_string("Coast Protect: OFF", x, y);
    info_window_update();
}
/* main_screen_refresh -- 0000:0ce0, 74 bytes */
extern void buffer_fill(char c, DosPort port);
extern void menu_draw_bar(void *m, void *sel, i16 update);
extern void draw_map_2(void);

void main_screen_refresh(void)
{
    buffer_fill(0, scr_work);
    mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
    menu_draw_bar(menu_bar, 0, 1);
    draw_map_2();
    info_window_draw();
}
/* add_item -- 0000:0d2a, 116 bytes */
extern i16 hspot_add(i16 x0, i16 y0, i16 x1, i16 y1, i16 v1, i16 v2, i16 id);
extern i16 hspot_which(u16 x, u16 y, i16 id);
extern i16 numspots;                                            /* DS:0x716 */
extern HotSpot spot[];                                          /* DS:0x530a */
extern i16 myspot;                                              /* DS:0x1ce */

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
/* selection_mouse -- 0000:11ee, 56 bytes */
extern i16 mouse_x;                                             /* DS:0x724 */
extern i16 mouse_y;                                             /* DS:0x726 */
extern i16 mouse_any_stroke;                                    /* DS:0x732 */

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
/* cursor_update -- 0000:1310, 244 bytes */
// MSC: #pragma optimize("e", on)
extern i16 cursor_on;                                           /* DS:0x6972 */
extern i16 map_corner_x;                                        /* DS:0x49f2 */
extern i16 map_corner_y;                                        /* DS:0x49f4 */
extern i16 map_view_size_x;                                     /* DS:0x52d4 */
extern i16 map_view_size_y;                                     /* DS:0x52d8 */
extern i16 map_tile_inset_x;                                    /* DS:0x5ad8 */
extern i16 map_tile_inset_y;                                    /* DS:0x5af4 */
extern i16 map_tile_size_x;                                     /* DS:0x4e8a */
extern i16 map_tile_size_y;                                     /* DS:0x4e8c */
extern i16 map_scale;                                           /* DS:0x4d0 */
extern DosPort scr_orig;                                        /* DS:0x3afc */
extern void *icons;                                         /* DS:0x68 */
extern void buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h);

/* Toggles the cursor: redraws the info window, restores the square from the backdrop and
 * draws the cursor icon (0x13 + zoom) over it when it is on. */
void cursor_update(void)
{
    i16 cx;
    i16 sx;
    i16 cy;
    i16 sy;
    i16 miny;
    i16 maxx;

    info_window_draw();
    cursor_on = !cursor_on;
    if (map_corner_x <= cursor_x && map_corner_x + map_view_size_x > cursor_x
        && map_corner_y <= cursor_y && map_corner_y + map_view_size_y > cursor_y) {
        sy = (cursor_y - map_corner_y + map_tile_inset_y) * map_tile_size_y;
        sx = (cursor_x - map_corner_x + map_tile_inset_x) * map_tile_size_x;
        buffer_rect_copy_2(sx, sy, sx, scr_orig, scr_work, sy + 8, map_tile_size_x, map_tile_size_y);
        if (cursor_on)
            sprite_draw(map_scale + 0x13, icons, &scr_work, sx, sy + 8);
        mouse_video_update(sx, sy + 8, sx, sy + 8, map_tile_size_x, map_tile_size_y);
    }
}
/* set_center -- 0000:1404, 90 bytes */
extern i16 on_map(i16 x, i16 y);
extern i16 map_center_x;                                        /* DS:0x4c8 */
extern i16 map_center_y;                                        /* DS:0x4ca */
extern i16 game_mode;                                           /* DS:0x70 */

/* Centres the view on (x, y); in the editor (game_mode 1) the cursor goes there too, with
 * the cursor toggled off and on around the move when flag is set. */
void set_center(register i16 x, register i16 y, i16 flag)
{
    if (on_map(x, y)) {
        map_center_x = x;
        map_center_y = y;
        if (game_mode == 1) {
            if (flag && cursor_on)
                cursor_update();
            cursor_x = x;
            cursor_y = y;
            if (flag) {
                cursor_on = 0;
                cursor_update();
            }
        }
        draw_map_2();
    }
}
/* possibly_center -- 0000:145e, 182 bytes */
#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))
extern i16 map_corner_x2;                                       /* DS:0x6026 */
extern i16 map_corner_y2;                                       /* DS:0x603e */

/* Re-centres on (x, y) when the rectangle (x, y)-(x2, y2) comes within 2 squares of the
 * view's edge and the view can still move that way. Returns 1 when it did. */
// MSC: #pragma optimize("e", off)
i16 possibly_center(i16 x, i16 y, i16 x2, i16 y2, i16 flag)
{
    i16 need;
    i16 maxx;
    i16 maxy;
    i16 minx;
    i16 miny;
    register i16 result;
    register i16 xr;

    xr = x;
    need = 0;
    maxx = ((x2) >= (xr) ? (x2) : (xr));
    miny = ((y2) <= (y) ? (y2) : (y));
    maxy = ((y2) >= (y) ? (y2) : (y));
    minx = ((x2) <= (xr) ? (x2) : (xr));
    if (minx < map_corner_x + 2 && map_corner_x > 1)
        need = 1;
    result = need;
    if (miny < map_corner_y + 2 && map_corner_y > 1)
        result = 1;
    if (map_corner_x2 - 2 < maxx && map_file.w - 2 > map_corner_x2)
        result = 1;
    if (map_corner_y2 - 2 < maxy && map_file.h - 2 > map_corner_y2)
        result = 1;
    if (result)
        set_center(xr, y, flag);
    return result;
}
/* move_cursor -- 0000:1514, 102 bytes */
// MSC: #pragma optimize("e", on)
/* Moves the cursor by (dx, dy) when that stays inside the map's border. */
void move_cursor(i16 dx, i16 dy)
{
    i16 ox;
    i16 oy;
    register i16 nx;
    register i16 ny;
    i16 ok;

    ok = 1;
    ox = cursor_x;
    oy = cursor_y;
    nx = ox + dx;
    ny = oy + dy;
    if (nx < 1 || ny < 1)
        ok = 0;
    if (map_file.w - 1 <= nx || map_file.h - 1 <= ny)
        ok = 0;
    if (ok) {
        possibly_center(ox, oy, ox, oy, 1);
        cursor_x = nx;
        cursor_y = ny;
        cursor_update();
    }
}
/* set_view_mode -- 0000:157a, 8 bytes */
void set_view_mode(void)
{
    game_mode = 1;
}
/* set_zoom_level -- 0000:1582, 74 bytes */
extern void compute_view_parameters(void);

void set_zoom_level(register i16 level)
{
    level = max(level, 0);
    level = min(level, 3);
    map_scale = level;
    compute_view_parameters();
    if (possibly_center(cursor_x, cursor_y, cursor_x, cursor_y, 0) == 0)
        draw_map_2();
}
/* memory_check -- 0000:15cc, 48 bytes */
typedef struct { char pad[0x4a]; i32 avail; } MenuArena;
extern i32 mem_get_avail(void);
extern i32 popup_subst_number[];                               /* DS:0x698a */
extern i16 popup_box(char *file, char *key, i16 sel);

/* The free memory and the menu arena's free count, in DEBUG.TXT's MEMORY popup. */
void memory_check(void)
{
    popup_subst_number[0] = mem_get_avail();
    popup_subst_number[1] = ((MenuArena *)menu_bar)->avail;
    popup_box("DEBUG", "MEMORY", 0);
}
/* create_me -- 0000:15fc, 116 bytes */
extern i16 popup_ask(char *txt, char *key, char *dflt, i16 max);
extern char popup_text_buffer[];                                /* DS:0x4b64 */
extern void fileio_new_ext(char *dst, char *src, char *ext);
extern char map_name[];                                         /* DS:0x4a18 */
extern i16 create_blank_map(i16 w, i16 h);
extern i16 map_changes;                                         /* DS:0x46 */

// MSC: #pragma optimize("e", off)
/* A new blank map: asks for its name (UNTITLED.MP), 0x3a x 0x48. 0 when created. Plain flags
 * (the dead wide/high stores stay, as volatile stores; the call takes the literals). */
i16 create_me(void)
{
    i16 high;
    i16 wide;
    char buf[0x50];
    register i16 result;

    result = 1;
    if (popup_ask("MAPEDIT", "NEWNAME", "UNTITLED.MP", 0x14) == 0) {
        msc_strcpy(buf, popup_text_buffer);
        fileio_new_ext(buf, buf, "MP");
        *(volatile i16 *)&wide = 0x3a;
        *(volatile i16 *)&high = 0x48;
        msc_strcpy(map_name, buf);
        if (create_blank_map(0x3a, 0x48) == 0) {
            map_changes = 1;
            result = 0;
        }
    }
    return result;
}
/* continent_check -- 0000:1670, 368 bytes */
// MSC: #pragma optimize("e", on)
extern i16 *sprite_base;                                    /* DS:0x8c */
extern u8 *continent;                            /* DS:0x4b0 */
extern DosPort scr_video;                                       /* DS:0x3b1c */
extern void * mem_get(i32 n);
extern i16 map_find_continents(void);
extern i16 buffer_init(DosPort *port, u16 w, u16 h);
extern i16 buffer_free(DosPort *port);
extern void buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h);

// MSC: #pragma optimize("elg", on)
/* Find Continents: numbers the land masses and oceans (map_find_continents, with a 0xfde8
 * byte scratch block in sprite_base), warns when there are more than 15 of either, and shows
 * the continent plane (low nibble, oceans black) on the screen until a key or click.
 * The two ports are one array (port[0], the aliased work buffer, sorts deepest); the plane
 * port's stores are written w, h, bits to come out h, bits, w ("elg" reverses them). */
void continent_check(void)
{
    i16 flags;
    i16 y;
    i16 x;
    u8 *p;
    DosPort port[2];

    sprite_base = mem_get(0xfde8L);
    if (sprite_base != 0) {
        flags = map_find_continents();
        mem_free(sprite_base);
        sprite_base = 0;
        if (flags & 1)
            popup_box("MAPEDIT", "CONTINENTS1", 0);
        if (flags & 2)
            popup_box("MAPEDIT", "CONTINENTS2", 0);
        port[1].w = map_file.w;
        port[1].h = map_file.h;
        port[1].bits = continent;
        buffer_init(&port[0], map_file.w, map_file.h);
        buffer_rect_copy(0, 0, map_file.w, port[1], port[0], map_file.h);
        for (y = 0; y < map_file.h; y++) {
            for (x = 0; x < map_file.w; x++) {
                p = (u8 *)port[0].bits + y * map_file.w + x;
                *p &= 0xf;
                if ((map_get(x, y) & 0x1f) == 0x19 || (map_get(x, y) & 0x1f) == 0x1a)
                    *p = 0;
            }
        }
        buffer_rect_copy(0, 0, map_file.w, port[0], scr_video, map_file.h);
        event_wait();
        mouse_video_update(0, 0, 0, 0, 0x140, 0xc8);
        buffer_free(&port[0]);
        map_changes = 1;
    }
}
/* execute_menu_event -- 0000:17e0, 932 bytes */
// MSC: #pragma optimize("elg", off)
// MSC: #pragma optimize("e", on)
typedef struct { char pad[0x42]; i16 game_in_progress; } GameState_;
extern GameState_ game;                                          /* DS:0x5e2c */
extern char popup_subst_string[][0x40];                         /* DS:0x634e */
extern i16 write_map_file(void);
extern i16 load_map_file(void);
extern void new_mini(void);

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
extern i16 map_pixel_size_x;                                    /* DS:0x64ec */
extern i16 map_pixel_size_y;                                    /* DS:0x64ee */

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
/* change_map -- 0000:1be0, 328 bytes */
extern u8 *map_loc(i16 x, i16 y);

/* Paints the selected terrain (and/or masks) onto square (x, y) inside the view, or with
 * pickup picks the square's terrain up as the selection. Ocean squares are left alone when
 * the coastline protect is on and a terrain is selected. */
void change_map(register i16 x, register i16 y, i16 pickup)
{
    u8 *p;
    u8 b;
    u8 t;
    u8 t2;

    if (x < 1)
        return;
    if (y < 1)
        return;
    if (map_file.w - 1 <= x)
        return;
    if (map_file.h - 1 <= y)
        return;
    if (map_corner_x > x)
        return;
    if (map_corner_x2 < x)
        return;
    if (map_corner_y > y)
        return;
    if (map_corner_y2 < y)
        return;
    p = map_loc(x, y);
    b = *p;
    t2 = t = b & 0x1f;
    if (pickup == 0 || selected_right_mouse_change) {
        if (!((t == 0x19 || t == 0x1a) && coastline_protect && selected_terrain >= 0)) {
            if (!((t == 0x19 || t == 0x1a) && (selected_physical_or_mask & 0x20))) {
                b &= selected_physical_and_mask;
                if (pickup == 0)
                    b |= selected_physical_or_mask;
            }
            if (selected_terrain >= 0 && pickup == 0)
                t = selected_terrain;
        }
    }
    if (pickup && selected_right_mouse_change == 0) {
        if (t == 0x19 || t == 0x1a) {
            selected_terrain = t;
            selected_physical_and_mask = 0x40;
        } else {
            selected_terrain = t;
            selected_physical_and_mask = 0xff;
        }
        selected_physical_or_mask = 0;
        info_window_draw();
        return;
    }
    b &= 0xe0;
    b |= selected_right_mouse_change ? t2 : t;
    if (*p != b) {
        *p = b;
        map_changes = 1;
    }
}
/* fill_map -- 0000:1d28, 142 bytes */
extern void draw_map_region_2(i16 x, i16 y, i16 w, i16 h);

// MSC: #pragma optimize("elg", on)
/* Paints (or picks up) a square of fill_radius around (x, y) and redraws it. */
void fill_map(i16 x, i16 y, i16 pickup)
{
    i16 x0;
    i16 y0;
    i16 size;
    i16 xx;
    i16 yy;

    x0 = x - fill_radius;
    y0 = y - fill_radius;
    size = fill_radius * 2 + 1;
    for (yy = y0; yy < y0 + size; yy++)
        for (xx = x0; xx < x0 + size; xx++)
            if (on_map(xx, yy))
                change_map(xx, yy, pickup);
    draw_map_region_2(x0 - 1, y0 - 1, size + 2, size + 2);
}
/* parse_main_keys -- 0000:1db6, 456 bytes */
// MSC: #pragma optimize("elg", off)
// MSC: #pragma optimize("e", on)
#include "msc.h"
extern i16 mykey;                                               /* DS:0x526c */

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
/* parse_area_map -- 0000:1f8e, 244 bytes */
extern i16 mouse_stroke_type;                                   /* DS:0x4e8e */
extern i16 mouse_current_area;                                  /* DS:0x5ace */
extern i16 mouse_start_stroke;                                  /* DS:0x728 */
extern i16 mouse_button;                                        /* DS:0x720 */

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
/* parse_area_mini -- 0000:2082, 94 bytes */
extern i16 mini_x;                                              /* DS:0x6982 */
extern i16 mini_y;                                              /* DS:0x49c2 */
extern i16 minimax(i16 v, i16 lo, i16 hi);

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
/* human_interface_loop -- 0000:2124, 376 bytes */
extern i32 game_clock;                                         /* DS:0x4a14 */
extern i32 cursor_clock;                                       /* DS:0x4ea0 */
extern i16 key_parsed;                                          /* DS:0x4a68 */
extern i16 menu_bar_mouse_parse(void *m, i16 mode);
extern i16 menu_bar_key_parse(void *m, i16 key);
extern i16 menu_key_parse(void *m, i16 key);

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
        if (any == 0 && ((MenuHead *)menu_bar)->f0 == 0) {
            menu_bar_mouse_parse(menu_bar, 1);
            if (((MenuHead *)menu_bar)->f0 == 0)
                done |= parse_mouse_stroke();
        }
        if (((MenuHead *)menu_bar)->f0 != 0) {
            done = 1;
            execute_menu_event(((MenuHead *)menu_bar)->f0);
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
/* computer_turn -- 0000:22ae, 2 bytes */
void computer_turn(void)
{
}
/* turn_control_loop -- 0000:22b0, 52 bytes */
extern void generate_mini(void);

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
/* load_terrain -- 0000:22e4, 82 bytes */
typedef struct { i16 name; char f2; char f3; char f4; char pad[2]; char v[9]; } TerrainRow;
extern i16 pad_terrain_view;      /* where a second view of `terrain` was declared: keeps the symbol count */
extern i16 text_item_string(void);
extern i16 text_item_number(void);

/* One terrain row from the text file: its name word, three numbers and nine values. */
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
/* load_data -- 0000:2336, 324 bytes */
// MSC: #pragma optimize("elg", on)
/* The terrain table from NAMES.TXT: 8 unforested rows, 8 forested (copied to rows 16-23),
 * 5 others, the 5 feature words, and the editor's colours from COLORS. */
void load_data(void)
{
    char name[16] = "names";
    char labels[16] = "labels";
    i16 i;
    TerrainRow *p;
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
    if (text_open(0, "COLORS") == 0) {
        text_get();
        basic_color = text_item_number();
        hilite_color = text_item_number();
        grey_color = text_item_number();
        enhance_color = text_item_number();
        shadow_color = text_item_number();
        select_color = text_item_number();
        border0 = text_item_number();
        border1 = text_item_number();
        border2 = text_item_number();
    }
    popups_normal();
}
/* start_new_game -- 0000:247a, 156 bytes */
// MSC: #pragma optimize("elg", off)
// MSC: #pragma optimize("e", on)
extern i16 map_selected;                                        /* DS:0x4c */
extern i16 create_me_now;                                       /* DS:0x54 */
extern i16 map_force;                                           /* DS:0x4d8 */
extern i16 map_error;                                           /* DS:0x4a4 */
extern i16 exit_value;                                          /* DS:0x62 */
extern i16 map_startup(i16 big);

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
extern void strings_init(i32 size);
extern void timer_install(void);
extern void timer_remove(void);
extern void screen_dominant_mode(i16 mode);
extern void video_init(i16 mode, i16 flag);
extern void mouse_init(i16 a, i16 b);
extern void mouse_show(void);
extern void mouse_hide(void);
extern void mcga_compute_retrace_parameters(void);
extern i16 pal_load_2(char *name, char *pal);
extern void mouse_video_init(void);
extern void video_update(DosPort *port, i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);
extern void *font_load(char *name);
extern void *font_inter;                                    /* DS:0x3a96 */
extern void popup_init(void *font, i16 size);
extern void mcga_setpal(char *pal);
extern i16 load_terrain_tiles(void);
extern void *sprite_series_load(char *name, i16 flags);
extern void *cursor;                                        /* DS:0x64 */
extern void *tiles2;                                        /* DS:0x4c4 */
extern DosPort *menu_tile;                                      /* DS:0x632 */
extern i16 menu_select_color_0, menu_bar_select_color_0;        /* DS:0x61c, 0x620 */
extern i16 menu_font_main_color, menu_font_grey_color, menu_font_hilite_color;   /* DS:0x626.. */
extern i16 menu_bar_font_main_color, menu_bar_font_grey_color, menu_bar_font_hilite_color;   /* DS:0x62c.. */
extern void get_tile_colors(void);
extern i16 initialize_map_memory(void);
extern i16 video_mode;                                          /* DS:0x49c6 */

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
    if (pal_load_2("viceroy.pal", (char *)0xa000fc00L)) {
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
    mcga_setpal((char *)0xa000fc00L);
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
    menu_select_color_0 = menu_bar_select_color_0 = select_color;
    menu_font_main_color = menu_bar_font_main_color = basic_color;
    menu_font_grey_color = menu_bar_font_grey_color = grey_color;
    menu_font_hilite_color = menu_bar_font_hilite_color = hilite_color;
    initialize_map_memory();
    map_undo_memory = mem_get(0x2ee0L);
    if (map_undo_memory != 0)
        undo_available = 1;
    if (start_new_game() == 0) {
        mcga_setpal((char *)0xa000fc00L);
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
// MSC: #pragma optimize("e", off)
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
extern i16 xms_disabled;                                        /* DS:0x3c6a */
extern void pack_enable_pfab(void);
extern void himem_startup(void);
extern void himem_shutdown(void);
extern i16 mads_mode;                                           /* DS:0x42 */
extern i16 file_set;                                            /* DS:0x44 */

// MSC: #pragma optimize("elg", on)
void editor_main(i16 argc, char **argv)
{
    register i16 i;
    char *s;

    xms_disabled = 1;
    pack_enable_pfab();
    himem_startup();
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
    himem_shutdown();
    if (exit_value)
        msc_printf("Exit value: %d\n", exit_value);
}

/* @@END@@ */

