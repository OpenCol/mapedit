/* popup.h -- popups: boxes of text, choices, check boxes, entry fields and sprites.
 *
 * Defined in ui/popup.c; theme.c's popups_normal sets the colours. popup_create makes an
 * empty popup, the popup_add_* functions add its parts, popup_layout sizes and places it,
 * popup_draw draws it and popup_exec runs it and returns the chosen key. popup_box and
 * popup_ask do all of that for one section of a text file (MAPEDIT.TXT in the editor);
 * pop, popi and the other short wrappers do it for the game's text file. popup.c describes
 * the text file's sections, the markup and the popup flags.
 */
#ifndef MAPEDIT_POPUP_H
#define MAPEDIT_POPUP_H

#include "msc.h"
#include "font.h"
#include "sprite.h"

#pragma pack(push, 2)
/* popup_add_cursor's record: the caller's value b and pointer a; the other words are unused.
 * The popup only tests whether it has one, and nothing in the editor adds one. */
typedef struct { i16 f0; i16 f2; i16 b; i16 f6; i16 f8; i16 fa; void *a; } DosAttach;
/* An entry field: flags (80h: the text is still the preset one, shown selected, and typing
 * replaces it), the label's width and the field's (len 'W's) in pixels, the most characters
 * it takes, its label (with a space added), the text typed, and the next field. */
typedef struct DosEdit {
    i16 flags; i16 label_w; i16 field_w; i16 len; char *label; char *buf;
    struct DosEdit *next;
} DosEdit;
/* A choice or a check box: flags (1 greyed, which an empty separator item also is; 2 drawn
 * in the highlight colour), its hotkey (the letter after '~', upper-cased, or a function
 * key's code), the key popup_exec returns for it, the check box's state, its text (a check
 * box's starts "~[ ", the bracket showing the state), two unused words and the list links. */
typedef struct DosItem {
    i16 flags; i16 hotkey; i16 key; i16 value; char *text; i16 fc; i16 fe;
    struct DosItem *next; struct DosItem *prev;
} DosItem;
/* A text line: flags (1 "^^": on its own line, centred; 2 "^": on its own line), the text
 * without its marks, and the next line. */
typedef struct DosLine { i16 flags; char *text; struct DosLine *next; } DosLine;
/* A sprite in the popup's column: x and x2 are the top of its row and of the next one,
 * relative to the content (vertical, despite the names); idx is the frame (with flag 02h, a
 * unit); key is returned when it is chosen; label is drawn to its right; sheet is its series.
 * The portrait reuses the record: idx is its x on the screen and x its y, and the King's
 * second sheet hangs on next. */
typedef struct DosSprite {
    i16 x; i16 x2; i16 idx; i16 key; char *label; SpriteSeries *sheet;
    struct DosSprite *next;
} DosSprite;
/* How a popup's text is drawn: extra pixels after each character, the plain colour, the
 * colour of greyed text, the alternate colour (hotkeys, { } spans, highlighted items), the
 * font's third and fourth colours, and the font. */
typedef struct { i16 spacing; i16 color; i16 hi_color; i16 alt_color; i16 c8; i16 ca; DosFont *font; } TextStyle;
/* A popup. Many fields are named by their position in the 1994 record; what they hold:
 *   f0               the result: the chosen item's or sprite's key, -1 when cancelled
 *   n_items .. n_edits   how many parts of each kind
 *   flags            the popup's flags (popup.c)
 *   pos_x, pos_y     the position asked for (-1: centred)
 *   x, y, w, h       the box on the screen
 *   r_x .. r_h       the box with its portrait: what popup_exec saves and restores
 *   item_w           the widest item; after layout, the content's width
 *   list_rows        the indent of an item's text (4), despite the name
 *   f24, f26         where the items and entry fields start (relative, then on the screen)
 *   width            the width text lines wrap to; after layout, the content's width
 *   f2a, f2c         where the text lines start
 *   spr_h            the widest sprite, despite the name
 *   spr_lw           the widest sprite label, with its spacing
 *   f32              the gap between a sprite and its label (4)
 *   edit_w           the widest entry field with its label
 *   f36, f38         a third origin, moved with the others and never read
 *   f3a              unused
 *   col0 .. col4     the background and the selection colours (only col0 and col2 are
 *                    used; col1 and col3 are passed along to popup_fill), the frame (unused)
 *   f46              the space between rows (3; 0 with flag 10h)
 *   f48              the margin beside the content (2; 0 with flag 10h)
 *   f4a              extra space above the content; always 0
 *   sel_item         the selected item
 *   first_sprite     the active sprite, despite the name
 *   items .. edits   the four lists of parts
 *   attach           popup_add_cursor's record, or 0
 *   dude, dude_name  the portrait's sprite and the name of its file
 *   f70              the last item, where Up wraps round to
 *   style            the text style
 *   arena            the arena the parts are allocated from, which follows the record */
typedef struct {
    i16 f0; i16 n_items; i16 n_lines; i16 n_sprites; i16 n_edits; i16 flags; i16 pos_x; i16 pos_y;
    i16 x; i16 y; i16 w; i16 h; i16 r_x; i16 r_y; i16 r_w; i16 r_h;
    i16 item_w; i16 list_rows; i16 f24; i16 f26; i16 width; i16 f2a; i16 f2c; i16 spr_h;
    i16 spr_lw; i16 f32; i16 edit_w; i16 f36; i16 f38; i16 f3a; i16 col0; i16 col1;
    i16 col2; i16 col3; i16 col4; i16 f46; i16 f48; i16 f4a;
    DosItem *sel_item; DosSprite *first_sprite; DosItem *items; DosLine *lines;
    DosSprite *sprites; DosEdit *edits; DosAttach *attach;
    DosSprite *dude; char *dude_name; void *f70;
    TextStyle style; Arena arena[1];   /* Port: the port's Arena, wider than 1994's */
} DosPopup;
#pragma pack(pop)

DosItem * Popup_find_item(i16 key, DosPopup *p);
i16 font_height(DosFont *c);
i16 hotkey_of(u8 *s);
i16 pop(char *key);
i16 pop2(char *key, i16 sel);
i16 pop2f(char *key, i16 sel, i16 nation);
i16 pop2i(char *key, i16 sel, i16 tribe);
i16 pop2m(char *key, i16 sel, i16 dude);
void pop_clear(void);
i16 pop_get(i16 n);
void pop_set(i16 bit, i16 on);
i16 popf(char *key, i16 nation);
i16 popi(char *key, i16 tribe);
i16 popk(char *key);
i16 popm(char *key, i16 dude);
DosItem *popup_add_check(DosPopup *p, char *s, i16 key, i16 on);
DosAttach *popup_add_cursor(DosPopup *p, void *a, i16 b);
DosEdit *popup_add_entry(DosPopup *p, char *label, char *init, i16 len);
DosItem *popup_add_item(DosPopup *p, char *s, i16 key);
DosSprite *popup_add_sprite(DosPopup *p, SpriteSeries *sheet, i16 idx, char *label, i16 key);
DosLine *popup_add_string(DosPopup *p, char *s);
i16 popup_ask(char *txt, char *key, char *dflt, i16 max);
i16 popup_ask_number(char *txt, char *key, i16 dflt);
void popup_big_sprite(DosPopup *p);
i16 popup_box(char *file, char *key, i16 sel);
void popup_box_draw(DosPopup *p, i16 x, i16 y, i16 w, i16 h);
void popup_country(i16 slot, i16 flag, i16 nation);
DosPopup *popup_create(i16 size, DosFont *font);
i16 popup_draw(DosPopup *p);
void popup_draw_edits(i16 full, DosPopup *p);
void popup_draw_items(i16 full, DosPopup *p);
i16 popup_draw_line(i16 x, i16 y, DosPopup *p, char *s);
void popup_draw_sprites(i16 labels, i16 full, i16 hilite, DosPopup *p);
void popup_dude_ms(DosPopup *p);
void popup_dude_myr(DosPopup *p);
void popup_dude_tribe(DosPopup *p);
i16 popup_exec(DosPopup *p);
void popup_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 sx, i16 sy, i16 c, char color, char color2, i16 d, i16 e);
void popup_grey(DosPopup *p, i16 key, i16 on);
void popup_help_draw(DosPopup *p);
void popup_hilite(DosPopup *p, i16 key, i16 on);
void popup_init(DosFont *font, i16 size);
i16 popup_layout(DosPopup *p);
void popup_load_dude(DosPopup *p);
void popup_num(i16 slot, i32 v);
void popup_off(void);
void popup_parse_string(char *src, char *dst);
i16 popup_read_check(DosPopup *p, i16 key);
void popup_read_colors(void);
void popup_release_all_grey(DosPopup *p);
void popup_say(i16 slot, i16 word);
void popup_say_string(i16 slot, char *s);
void popup_set_active_item(DosPopup *p, i16 key);
void popup_set_active_sprite(DosPopup *p, DosSprite *spr);
void popup_set_dude_name(char *name, DosPopup *p);
void popup_set_font(TextStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi_color, i16 alt_color, i16 c8, i16 ca);
void popup_set_width(DosPopup *p, i16 w);
void popup_show(DosPopup *p);
DosPopup * popup_start_box(char *txt, char *key, i16 sel);
i16 popup_wrap_lines(i16 draw, DosPopup *p);
void popup_write_check(DosPopup *p, i16 key, i16 v);
void style_color(i16 hi, i16 alt, TextStyle *st);
i16 style_draw_text(i16 x, i16 y, i16 flags, TextStyle *st, char *s);
i16 style_text_width(TextStyle *st, char *s);

/* the portraits (the game's; nothing in the editor sets them) */
extern i16 king_active;
extern i32 king_clock;
extern i16 king_frame;
extern i16 orig_flag;                  /* a frameless (10h) popup is running */
/* the colours, palette indices: each *_1 is passed along and not used */
extern i16 popup_background_color_0;
extern i16 popup_background_color_1;
extern i16 popup_border_color;
extern i16 popup_border_color_down;
extern i16 popup_border_color_up;
extern i16 popup_checkbox;             /* check-box states: bit n - 1 for item n */
extern i16 popup_enhance_color;
extern i16 popup_flags;                /* the next popup's flags */
extern DosFont *popup_font;            /* the font popup_start_box uses */
extern i16 popup_font_grey_color;
extern i16 popup_font_hilite_color;
extern i16 popup_font_main_color;
extern i16 popup_foreign;
extern i16 popup_heap;                 /* the arena size popup_start_box asks for */
extern i16 popup_help;
extern i16 popup_help_me;
extern i16 popup_hiliting;
extern i16 popup_indian;
extern i16 popup_messenger;
extern i16 popup_no_draw;
extern i16 popup_number;
extern i16 popup_pal;
extern i16 popup_select_color_0;
extern i16 popup_select_color_1;
extern i16 popup_shadow_color;
extern i16 popup_start_ask;
extern i16 popup_start_max_len;
extern char *popup_start_string;
extern i32 popup_subst_number[9];      /* %NUMBER0..8 and %HEX0..8 */
extern char popup_subst_string[5][0x40];   /* %STRING0..4 */
extern char popup_text_buffer[80];     /* the last entry field's text */
extern DosPort *popup_tile;            /* the background tile drawn for colour 7 */
extern i16 popup_x;                    /* the next popup's position (-1: centred) */
extern i16 popup_y;
extern i16 sprite_tight;               /* the main sprites were freed for a portrait */

#endif
