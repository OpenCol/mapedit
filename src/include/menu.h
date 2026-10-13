/* menu.h -- the functions, data and records of menu.c. */
#ifndef MAPEDIT_MENU_H
#define MAPEDIT_MENU_H

#include "msc.h"
#include "font.h"

#pragma pack(push, 2)
typedef struct { i16 spacing; i16 color; i16 hi; i16 alt; DosFont *font; } MenuStyle;
typedef struct MenuEntry {
    i16 flags; i16 hotkey; i16 id; char *text; i16 fa; i16 fc;
    struct MenuEntry *next; struct MenuEntry *prev;
} MenuEntry;
typedef struct MenuSub {
    i16 n_items; i16 x; i16 w; i16 drop_w; i16 hotkey; i16 id; i16 flags; char *name;
    struct Menu *menu; struct MenuSub *next; struct MenuSub *prev;
    MenuEntry *items;
} MenuSub;
typedef struct Menu {
    i16 f0; i16 n_subs; i16 bar_h; i16 title_gap; i16 item_gap; i16 gap; i16 item_pad;
    i16 bar_col; i16 bar_col2; i16 drop_col; i16 drop_col2; i16 sel_col; i16 sel_col2;
    i16 title_col; i16 title_col2; i16 frame_col;
    MenuStyle bar; MenuStyle drop; MenuSub *subs; Arena arena[1];   /* PORT: the port's Arena (0x12 bytes in 1994) */
} Menu;
#pragma pack(pop)

MenuSub *menu_add_bar_item(Menu *m, char *name, i16 id, i16 right);
MenuEntry *menu_add_item(Menu *m, i16 sub_id, char *name, i16 id);
void menu_bar_hide(Menu *m, i16 id, i16 on);
MenuSub *menu_bar_item(Menu *m, i16 id);
i16 menu_bar_key_parse(Menu *m, i16 key);
i16 menu_bar_mouse_parse(Menu *m, i16 mode);
void menu_bar_run(MenuSub *sub);
void menu_compute_size(MenuSub *sub, i16 *px, i16 *py, i16 *pw, i16 *ph, i16 *ix, i16 *iy);
Menu *menu_create(i16 size, DosFont *font);
void menu_draw_bar(Menu *m, MenuSub *sel, i16 update);
void menu_draw_menu(MenuSub *sub, MenuEntry *sel);
void menu_fill_rect(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 a, i16 b, i16 c, char color, char color2, i16 d, i16 e);
i16 menu_font_height(DosFont *f);
i16 menu_get_hotkey(u8 *s);
void menu_grey(Menu *m, i16 id, i16 on);
MenuEntry *menu_item(Menu *m, i16 id);
i16 menu_key_parse(Menu *m, i16 key);
void menu_off(Menu *m, i16 id, i16 on);
void menu_read_colors(void);
void menu_release_all_grey(Menu *m);
void menu_release_all_off(Menu *m);
void menu_set_font(MenuStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi, i16 alt);
i16 menu_string_width(MenuStyle *st, char *s);
i16 menu_string_write(MenuStyle *st, char *s, i16 x, i16 y, i16 hilite);

extern i16 menu_background_color_0;
extern i16 menu_background_color_1;
extern i16 menu_bar_background_color_0;
extern i16 menu_bar_background_color_1;
extern i16 menu_bar_font_grey_color;
extern i16 menu_bar_font_hilite_color;
extern i16 menu_bar_font_main_color;
extern i16 menu_bar_select_color_0;
extern i16 menu_bar_select_color_1;
extern i16 menu_border_color;
extern i16 menu_font_grey_color;
extern i16 menu_font_hilite_color;
extern i16 menu_font_main_color;
extern i16 menu_select_color_0;
extern i16 menu_select_color_1;
extern DosPort *menu_tile;

#endif
