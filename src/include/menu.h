/* menu.h -- menus: the menu bar along the top of the screen and its drop-downs.
 *
 * Defined in ui/menu.c. A Menu is the bar: a list of submenus (MenuSub), the titles on the
 * bar, each with a list of items (MenuEntry). menu_create allocates the Menu with an arena
 * after it, from which menu_add_bar_item and menu_add_item carve the records and their text.
 * A title or item's text may mark its hotkey letter with '~'; an item with empty text is a
 * separator line. menu_bar_key_parse, menu_key_parse and menu_bar_mouse_parse take a key or
 * the mouse; a chosen item's id lands in the Menu's f0. The colours below are palette
 * indices, copied into each Menu when it is created: the editor sets them from its theme
 * (theme.h) at startup.
 */
#ifndef MAPEDIT_MENU_H
#define MAPEDIT_MENU_H

#include "msc.h"
#include "font.h"

#pragma pack(push, 2)
/* How a menu's text is drawn: extra pixels after each character, the plain colour, the
 * colour of a greyed item, the colour of the ~hotkey letter, and the font. */
typedef struct { i16 spacing; i16 color; i16 hi; i16 alt; DosFont *font; } MenuStyle;
/* An item of a drop-down: flags (1 greyed, which an empty separator item also is; 2 hidden),
 * its hotkey (the letter after '~', upper-cased, or a function key's code for "~F~n"), the
 * id execute_menu_event receives, its text, two unused words, and the list links. */
typedef struct MenuEntry {
    i16 flags; i16 hotkey; i16 id; char *text; i16 fa; i16 fc;
    struct MenuEntry *next; struct MenuEntry *prev;
} MenuEntry;
/* A title on the bar and its drop-down: the number of items, the title's x and width on the
 * bar, the drop-down's width (the widest item plus padding), its hotkey, its id, flags (1
 * hidden), its text, the Menu it belongs to, the list links and the first item. */
typedef struct MenuSub {
    i16 n_items; i16 x; i16 w; i16 drop_w; i16 hotkey; i16 id; i16 flags; char *name;
    struct Menu *menu; struct MenuSub *next; struct MenuSub *prev;
    MenuEntry *items;
} MenuSub;
/* The menu bar. f0 is the id of the item last chosen, 0 for none; n_subs counts the titles.
 * The spacing, in pixels: bar_h above the titles, title_gap between titles, gap on either
 * side of a title's text, item_gap between items and item_pad on either side of an item's.
 * The colours: the bar's background (bar_col), a drop-down's (drop_col), behind the
 * selected item (sel_col) and behind the open title (title_col), and the drop-down's frame
 * (frame_col); each *2 colour is passed along to menu_fill_rect, which does not use it. bar
 * and drop are the text styles of the titles and the items; subs is the list of titles, and
 * arena holds the records and their text. */
typedef struct Menu {
    i16 f0; i16 n_subs; i16 bar_h; i16 title_gap; i16 item_gap; i16 gap; i16 item_pad;
    i16 bar_col; i16 bar_col2; i16 drop_col; i16 drop_col2; i16 sel_col; i16 sel_col2;
    i16 title_col; i16 title_col2; i16 frame_col;
    MenuStyle bar; MenuStyle drop; MenuSub *subs; Arena arena[1];   /* Port: the port's Arena,
                                                                       wider than 1994's */
} Menu;
#pragma pack(pop)

MenuSub *menu_add_bar_item(Menu *m, char *name, i16 id, i16 right);
MenuEntry *menu_add_item(Menu *m, i16 sub_id, char *name, i16 id);
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
i16 menu_key_parse(Menu *m, i16 key);
void menu_set_font(MenuStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi, i16 alt);
i16 menu_string_width(MenuStyle *st, char *s);
i16 menu_string_write(MenuStyle *st, char *s, i16 x, i16 y, i16 hilite);

/* the colours; each *_1 is the second colour menu_fill_rect is given and does not use */
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
extern DosPort *menu_tile;      /* the backdrop tile drawn wherever colour 7 is asked for */

#endif
