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

typedef struct { i16 h; i16 w; void *bits; } DosPort;
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
    return *(u8 *)0x00400017L & 3;
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
typedef struct { i16 name; char rest[14]; } TerrainRec;
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

