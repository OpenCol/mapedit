/* popup.c -- popups: boxes of text, choices, check boxes, entry fields and sprites.
 *
 * A popup (DosPopup) is one memory block: the record, then an arena its parts are allocated
 * from (popup_create), so freeing the block frees everything. The parts are four linked lists,
 * each kept in the order added:
 *   lines    text, word-wrapped to the popup's text width (popup_add_string);
 *   items    choices or check boxes, one per row (popup_add_item, popup_add_check);
 *   edits    entry fields (popup_add_entry);
 *   sprites  icons in a column at the left, each with an optional label (popup_add_sprite).
 * The box stacks the text, the items and the entry fields, with the sprite column beside them.
 * A popup has items or entry fields, not both: popup_layout drops the fields if it has both.
 *
 * popup_layout sizes and places the box, popup_draw draws it into scr_work and copies it to the
 * screen, and popup_exec runs it: it saves what the box covers, polls the mouse and the keyboard
 * until a choice is made, restores the screen and returns the chosen key. popup_box does the
 * whole job for one section of a text file: popup_start_box reads the section and builds the
 * popup, popup_exec runs it, and the block is freed. popup_ask does the same with an entry field
 * and leaves the text typed in popup_text_buffer. pop, popi and the other short wrappers use the
 * game's text file (game_text_name); the editor itself calls popup_box and popup_ask with
 * MAPEDIT.TXT.
 *
 * A section of the text file starts at its @KEY line: text lines, an empty line, the options
 * (one item per line, numbered from 1; for popup_ask, the entry field's label), and an empty
 * line or any unknown @ line to end it. Directives may appear anywhere in it, in any case:
 *   @TEXT, @OPTIONS, @PROMPT  switch to text or to options;
 *   @SMALLFONT                draw in menu_font;
 *   @X, @Y, @WIDTH            the position and the text width, from the first number on the
 *                             line ("@width=190"); @X and @Y match any line starting so;
 *   @LENGTH                   the entry field's length, unless the caller gave one;
 *   @CHECKBOX                 the items are check boxes;
 *   @DEFAULT                  for popup_ask, "=text", the field's text unless the caller gave
 *                             one; otherwise the number of the item preselected when the caller
 *                             passed 0.
 *
 * Markup in text lines and items:
 *   ^ at the start       the line stands on its own instead of joining the paragraph;
 *   ^^ at the start      the same, centred;
 *   ~c                   c is drawn in the other colour and is the item's hotkey; "~F~n" makes
 *                        function key Fn the hotkey;
 *   { ... }              drawn in the alternate colour (popup_font_hilite_color);
 *   |                    in an item, the rest is right-aligned;
 *   an empty item        a separator line, never selectable;
 *   %STRINGn, %NUMBERn, %HEXn, %COUNTRY, %YEAR, %%   substitutions (popup_parse_string).
 * Keys are keys_get's codes: ASCII, or 100h plus the scan code for an extended key (13Bh F1,
 * 148h Up, 150h Down, 153h Delete).
 *
 * A popup's flags:
 *   01h  a click outside the box closes it with nothing chosen (set for check boxes);
 *   02h  a unit picker: each sprite's idx is a unit, drawn with piece_draw at 16x16;
 *   04h  the items are check boxes; their states go in and out through popup_checkbox;
 *   08h  popup_exec does not save and restore what the box covers;
 *   10h  no frame and no margins; while it runs (orig_flag) the background comes from scr_orig;
 *   20h  draw only: popup_exec draws it and returns;
 *   40h  set by popup_layout: the portrait does not fit on the screen and is not drawn;
 *   80h  keyboard mode: the active sprite gets a white box (set by a key, and from the start
 *        when there is no mouse driver).
 * popup_flags, popup_x and popup_y preset the flags and the position of the next popup created.
 *
 * Portraits ("dudes"): popup_indian, popup_messenger or popup_foreign, set before a popup runs,
 * give it a portrait beside or above the box: the King (popup_indian above 7; a second sheet
 * animates it), a tribe's chief, a messenger or a foreign nation's envoy. popup_exec loads the
 * sheets, frees them and resets the three to -1. The portraits, %COUNTRY and the unit picker
 * come from the game, which shares this module; in the editor nothing sets them, and the game
 * functions they call are the empty stand-ins in game.c.
 *
 * Port: the records hold 64-bit pointers, so each part is allocated at its own sizeof and the
 * arena is twice the size the caller asks for (no record is more than twice its 1994 size).
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "game.h"
#include "menu.h"
#include "theme.h"
#include "buffer.h"
#include "error.h"
#include "font.h"
#include "hspot.h"
#include "keys.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mcga.h"
#include "mem.h"
#include "mouse.h"
#include "pal.h"
#include "popup.h"
#include "screen.h"
#include "text.h"
#include "timer.h"
#include "units.h"
#include "w16types.h"
#include "sprite.h"

#include "msc.h"
#include "msc.h"
#include "msc.h"
#include "records.h"

/* The popup settings. The colours are defaults; popup_read_colors reads seven of them from the
 * TEXTCOLR sheet. Each background and selection colour comes as a pair, and only the first of a
 * pair is used.
 */
i16 popup_background_color_0 = 7;       /* the box (7: tiled with popup_tile) */
i16 popup_background_color_1 = 7;
i16 popup_select_color_0 = 8;           /* the selected item's bar */
i16 popup_select_color_1 = 8;
i16 popup_border_color = 0;             /* the frame's middle line */
i16 popup_border_color_up = 0;          /* the bevel's top and right edges */
i16 popup_border_color_down = 0;        /* the bevel's left and bottom edges */
i16 popup_font_main_color = 0;          /* text */
i16 popup_font_grey_color = 8;          /* greyed items */
i16 popup_font_hilite_color = 0xf;      /* {} spans, ~ hotkeys, hilited items */
i16 popup_enhance_color = 7;            /* the font's third colour */
i16 popup_shadow_color = 8;             /* the font's fourth colour */
i16 popup_checkbox = 0;                 /* check-box states: bit n-1 for item n */
i16 popup_flags = 0;                    /* the next popup's flags */
i16 popup_x = -1;                       /* the next popup's position (-1: centred) */
i16 popup_y = -1;
i16 popup_indian = -1;                  /* chief portrait: a tribe, 8 the King */
i16 popup_messenger = -1;               /* messenger portrait */
i16 popup_foreign = -1;                 /* foreign portrait */
i16 popup_hiliting = 0;                 /* drawing in the alternate colour */
i16 popup_pal = 1;                      /* popup_exec sets the palette first */
i16 popup_help = 0;                     /* show the help word; F1 asks for help */
i16 popup_help_me = 0;                  /* set when the user asked for help */
i16 popup_no_draw = 0;                  /* draw-only popups stay off the screen */
DosPort *popup_tile = 0;                /* the background pattern (scr_back) */
i16 king_active = 0;                    /* the King's portrait animates */
i16 sprite_tight = 0;                   /* main sprites freed for a portrait (1, 2) */

/* Copies the portrait sheet's name into the popup's arena. */
void popup_set_dude_name(char *name, DosPopup *p)
{
    p->dude_name = heap_get((i32)(msc_strlen(name) + 1), p->arena);
    msc__fstrcpy(p->dude_name, name);
}

/* Names the chief portrait for popup_indian: KING, which also starts the King's animation (its
 * first frame due in 240 ticks), or IND<t>A<m> for tribe t, m being piss_factor of the tribe's
 * anger at the player.
 */
void popup_dude_tribe(DosPopup *p)
{
    i16 mood;
    char name[20];

    if (popup_indian > 7) {
        msc_strcpy(name, "KING");
        king_frame = king_active = 1;
        king_clock = timer_read() + 240;
    } else {
        mood = piss_factor(pissed(popup_indian, game.player_nation));
        msc_strcpy(name, "IND0A0");
        name[3] += popup_indian;
        name[5] += mood;
    }
    popup_set_dude_name(name, p);
}

/* Names the messenger portrait: MSS<n>, n = popup_messenger. */
void popup_dude_ms(DosPopup *p)
{
    char name[20];

    msc_strcpy(name, "MSS0");
    name[3] += popup_messenger;
    popup_set_dude_name(name, p);
}

/* Names the foreign portrait: MYR<n>, n = popup_foreign. */
void popup_dude_myr(DosPopup *p)
{
    char name[20];

    msc_strcpy(name, "MYR0");
    name[3] += popup_foreign;
    popup_set_dude_name(name, p);
}

/* Loads the portrait sheet named by popup_set_dude_name into p->dude and, for the King, the
 * animation sheet "<name>2" into p->dude->next. p->dude stays 0 without a name and becomes 0
 * when a sheet fails to load. The sheet's palette goes into the master palette except the first
 * 16 colours, which are kept, and the palette is set. When memory is short the main sprites are
 * freed and the sheet is loaded again (sprite_tight 1, or 2 when only the King's second sheet
 * needed it). Colour cycling is paused meanwhile; if the first sheet fails, the original skips
 * cycle_save(0) and leaves the palette unset.
 */
void popup_load_dude(DosPopup *p)
{
    DosSprite *second;
    DosSprite *dd;
    i16 old;
    i16 king;
    char colors[0x30];
    char fname[0x50];

    if (p->dude_name == 0)
        return;
    king = popup_indian > 7 ? 1 : 0;
    p->dude = heap_get((i32)sizeof(DosSprite), p->arena);   /* Port: sizeof */
    dd = p->dude;
    dd->next = 0;
    if (king) {
        second = heap_get((i32)sizeof(DosSprite), p->arena);   /* Port: sizeof */
        dd->next = second;
    }
    msc__fstrcpy(fname, p->dude_name);
    old = cycling_active;
    cycling_active = 0;
    cycle_save(1);
    msc__fmemcpy(colors, (char *)(msc_vga + 0xfc00), 0x30);
    put_palette = (char *)(msc_vga + 0xfc00);
    dd->sheet = sprite_series_load(fname, 0);
    if (dd->sheet == 0) {
        reset_sprite_memory();
        sprite_tight = 1;
        dd->sheet = sprite_load(fname, 0);
    }
    msc__fmemcpy((char *)(msc_vga + 0xfc00), colors, 0x30);
    put_palette = 0;
    if (dd->sheet == 0) {
        p->dude = 0;
    } else {
        if (king) {
            msc_strcat(fname, "2");
            if (sprite_tight == 0)
                second->sheet = sprite_series_load(fname, 0);
            if (second->sheet == 0 && sprite_tight == 0) {
                sprite_tight = 2;
                reset_sprite_memory();
            }
            if (sprite_tight != 0)
                second->sheet = sprite_load(fname, 0);
        }
        cycle_save(0);
        if (king && second->sheet == 0) {
            if (sprite_tight != 1)
                mem_free(dd->sheet);
            p->dude = 0;
        }
        mcga_setpal((char *)(msc_vga + 0xfc00));
    }
    put_palette = 0;
    cycling_active = old;
}

i16 orig_flag = 0;                      /* a frameless (10h) popup is running */

/* Fills a rectangle of port with color. Colour 7, when popup_tile is set, is the textured
 * background instead: popup_tile tiled from the popup's corner (sx, sy), or, while a frameless
 * popup runs, the same rectangle copied from scr_orig. c, color2, d and e are not used.
 */
void popup_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 sx, i16 sy, i16 c,
                               char color, char color2, i16 d, i16 e)
{
    if (popup_tile != 0 && color == 7) {
        if (orig_flag != 0) {
            buffer_rect_copy(x, y, w, scr_orig, port, h);
            return;
        }
        buffer_tile(port, *popup_tile, x, y, w, h, sx, sy);
        return;
    }
    buffer_rect_fill(x, y, w, port, h, color);
}

/* Sets %STRING<slot> to s. */
void popup_say_string(i16 slot, char *s)
{
    msc__fstrcpy(popup_subst_string[slot], s);
}

/* Sets %STRING<slot> to string `word` of the string store. */
void popup_say(i16 slot, i16 word)
{
    popup_say_string(slot, strings(word));
}

/* Sets %STRING<slot> to the name say_country gives for nation (empty in the editor). */
void popup_country(i16 slot, i16 flag, i16 nation)
{
    char buf[0x50];

    buf[0] = 0;
    say_country(nation, flag, buf);
    popup_say_string(slot, buf);
}

/* Sets %NUMBER<slot> and %HEX<slot> to v. */
void popup_num(i16 slot, i32 v)
{
    popup_subst_number[slot] = v;
}

/* Fills text style st: the font, the gap after each character, and the colours for plain text,
 * greyed text (hi_color), {} spans and ~ hotkeys (alt_color), and the font's third and fourth
 * colours (c8, ca).
 */
void popup_set_font(TextStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi_color,
                i16 alt_color, i16 c8, i16 ca)
{
    st->spacing = spacing;
    st->color = color;
    st->hi_color = hi_color;
    st->alt_color = alt_color;
    st->c8 = c8;
    st->ca = ca;
    st->font = font;
}

/* Returns the width of s in style st, not counting the markup characters ~ { } |. The text after
 * a | counts. s must fit in 80 bytes.
 */
i16 style_text_width(TextStyle *st, char *s)
{
    char tmp[0x50];
    u8 *s2;
    i16 w;

    msc__fstrcpy(tmp, s);
    for (s2 = tmp; *s2 != 0; ) {
        if (msc_strchr("~{}|", *s2) != 0)
            msc_strcpy(s2, s2 + 1);
        else
            s2++;
    }
    w = font_string_width(st->spacing, st->font, tmp);
    return w;
}

/* Sets the font colours for greyed (hi), alternate (alt) or plain text in style st; hi wins. */
void style_color(i16 hi, i16 alt, TextStyle *st)
{
    if (hi)
        font_set_colors(-1, st->hi_color, st->c8, st->ca);
    else if (alt)
        font_set_colors(-1, st->alt_color, st->c8, st->ca);
    else
        font_set_colors(-1, st->color, st->c8, st->ca);
}

/* Draws s at (x, y) in scr_work and returns the x after it. ~ draws the next character in the
 * other colour, { and } switch the alternate colour on and off (popup_hiliting), and | ends the
 * text. Bit 0 of flags draws it greyed. A ~ at the very end makes it read past the terminator,
 * as the original does.
 */
i16 style_draw_text(i16 x, i16 y, i16 flags, TextStyle *st, char *s)
{
    char one[2];
    i16 hl;
    char *s2;

    hl = flags & 1;
    one[1] = 0;
    style_color(hl, popup_hiliting, st);
    for (s2 = s; *s2 != 0; ) {
        switch (*s2) {
        case '~':
            style_color(hl, popup_hiliting == 0, st);
            s2++;
            one[0] = *s2;
            x = font_write(&scr_work, x, y, st->font, one, st->spacing) + st->spacing;
            style_color(hl, popup_hiliting, st);
            break;
        default:
            one[0] = *s2;
            x = font_write(&scr_work, x, y, st->font, one, st->spacing) + st->spacing;
            break;
        case '{':
            popup_hiliting = 1;
            style_color(hl, popup_hiliting, st);
            break;
        case '}':
            popup_hiliting = 0;
            style_color(hl, popup_hiliting, st);
            break;
        case '|':
            return x;
        }
        s2++;
    }
    return x;
}

/* Returns an item's hotkey: the character after the last ~, upper-cased, or with "~F~n" the
 * function key Fn (10Ah plus the digit's code, so 13Bh for F1); 0 when there is no ~.
 */
i16 hotkey_of(u8 *s)
{
    i16 r;
    u8 *last;
    u8 *first;

    r = 0;
    last = msc__fstrrchr(s, '~');
    first = msc__fstrchr(s, '~');
    if (first != last && first[1] == 'F' && isdigit(last[1]))
        r = last[1] + 0x10a;
    else if (last != 0) {
        r = islower(last[1]) ? _toupper(last[1]) : last[1];
    }
    return r;
}

/* Allocates a popup with an arena of `size` bytes for its parts, its text in font and the popup
 * colours. It takes popup_flags, popup_x and popup_y and resets them. The defaults: a text width
 * of 80, items indented 4, and a 3-pixel row gap and 2-pixel margin (none when frameless);
 * without a mouse driver it starts in keyboard mode. Also clears king_active and sprite_tight.
 * Returns 0 when memory runs out.
 */
DosPopup *popup_create(i16 size, DosFont *font)
{
    DosPopup *result;
    char *m;   /* Port: a pointer; 1994 kept the address in a long */
    DosPopup *dp;
    void *a;
    void *e;

    result = 0;
    king_active = 0;
    sprite_tight = 0;
    /* Port: the record at its own sizeof, and the arena twice the size asked for: its records
     * hold pointers, now twice as wide, and none is more than twice its 1994 size.
     */
    m = (char *)mem_get((i32)(2 * (i32)(u16)size + sizeof(DosPopup)));
    if (m == 0)
        goto skip;
    {
        dp = (DosPopup *)m;
        e = m + sizeof(DosPopup);   /* Port: the arena follows the record */
        a = dp->arena;
        heap_declare(0x29, a, e, 2 * (i32)(u16)size);   /* Port: the doubled size */
        dp->sel_item = 0;
        dp->first_sprite = 0;
        (*(volatile i16 *)&dp->flags) = popup_flags;
        dp->pos_x = popup_x;
        dp->pos_y = popup_y;
        popup_y = popup_x = -1;
        dp->width = 0x50;
        dp->f32 = dp->list_rows = 4;
        dp->col0 = popup_background_color_0;
        dp->col1 = popup_background_color_1;
        dp->col2 = popup_select_color_0;
        dp->col3 = popup_select_color_1;
        dp->col4 = popup_border_color;
        dp->f46 = (dp->flags & 0x10) ? 0 : 3;
        dp->f48 = (dp->flags & 0x10) ? 0 : 2;
        dp->items = 0;
        dp->lines = 0;
        dp->sprites = 0;
        dp->edits = 0;
        dp->f70 = 0;
        dp->attach = 0;
        dp->dude = 0;
        dp->dude_name = 0;
        dp->f0 = 0;
        dp->n_items = 0;
        dp->n_lines = 0;
        dp->n_sprites = 0;
        dp->n_edits = 0;
        popup_flags = 0;
        dp->item_w = 0;
        dp->f24 = 0;
        dp->f26 = 0;
        dp->f2a = 0;
        dp->f2c = 0;
        dp->spr_h = 0;
        dp->spr_lw = 0;
        dp->edit_w = 0;
        dp->f36 = 0;
        dp->f38 = 0;
        dp->f4a = 0;
        popup_set_font(&dp->style, font, 0, popup_font_main_color, popup_font_grey_color, popup_font_hilite_color, popup_enhance_color,
                   popup_shadow_color);
        if (mouse_driver == 0)
            dp->flags |= 0x80;
        result = dp;
    }
skip:
    /* Never true: result is m whenever m is set. */
    if (m != 0 && (DosPopup *)m != result)
        mem_free((void *)m);
    return result;
}

/* Returns the item whose key is `key`, or 0. */
DosItem * Popup_find_item(i16 key, DosPopup *p)
{
    i16 found;
    DosItem *it;
    DosItem *result;

    found = 0;
    result = 0;
    it = p->items;
    while (found == 0) {
        if (it == 0)
            break;
        if (it->key == key) {
            found = 1;
            result = it;
        } else
            it = it->next;
    }
    return result;
}

/* Greys item `key` (on) or ungreys it; a greyed item cannot be selected. The item must exist. */
void popup_grey(DosPopup *p, i16 key, i16 on)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (on)
        it->flags |= 1;
    else
        it->flags &= ~1;
}

/* Draws item `key` in the alternate colour (on) or not. The item must exist. */
void popup_hilite(DosPopup *p, i16 key, i16 on)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (on)
        it->flags |= 2;
    else
        it->flags &= ~2;
}

/* Ungreys every item. */
void popup_release_all_grey(DosPopup *p)
{
    DosItem *it;

    it = p->items;
    while (it != 0) {
        it->flags &= ~1;
        it = it->next;
    }
}

/* Returns item `key`'s check-box state, 0 when there is no such item. */
i16 popup_read_check(DosPopup *p, i16 key)
{
    i16 r;
    DosItem *it;

    r = 0;
    it = Popup_find_item(key, p);
    if (it != 0)
        r = it->value;
    return r;
}

/* Sets item `key`'s check-box state, if the item exists. */
void popup_write_check(DosPopup *p, i16 key, i16 v)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (it != 0)
        it->value = v;
}

/* Selects item `key` (nothing when there is no such item). */
void popup_set_active_item(DosPopup *p, i16 key)
{
    p->sel_item = Popup_find_item(key, p);
}

/* Appends an item with text s and key `key`, which popup_exec returns when it is chosen, and
 * returns it. In a check-box popup the text gets the prefix "~[ ": its second character shows
 * the state ('[' off, ']' on) in the other colour. An empty s is a greyed separator. Widens the
 * item column (item_w) to fit, a | adding room for the gap.
 */
DosItem *popup_add_item(DosPopup *p, char *s, i16 key)
{
    DosItem *item;
    DosItem *prev;
    i16 mark;
    char box[0x14];
    i16 w;

    item = p->items;
    prev = 0;
    while (item != 0) {
        prev = item;
        item = item->next;
    }
    item = heap_get((i32)sizeof(DosItem), p->arena);   /* Port: sizeof */
    if (prev != 0) {
        prev->next = item;
        item->prev = prev;
    } else {
        p->sel_item = p->items = item;
        item->prev = 0;
    }
    p->f70 = item;
    if (p->flags & 4) {
        mark = 3;
        msc_strcpy(box, "~[ ");
    } else {
        mark = 0;
        box[0] = 0;
    }
    item->next = 0;
    item->flags = 0;
    item->value = 0;
    item->text = heap_get((u32)(msc__fstrlen(s) + mark + 1), p->arena);
    msc__fstrcpy(item->text, box);
    msc__fstrcat(item->text, s);
    if (*s == 0)
        item->flags |= 1;
    item->key = key;
    item->hotkey = hotkey_of(s);
    w = style_text_width(&p->style, item->text);
    w += p->f48 * 2 + p->list_rows;
    if (msc__fstrchr(item->text, '|') != 0)
        w += p->list_rows;
    p->item_w = max(p->item_w, w);
    p->n_items++;
    return item;
}

/* Appends a check-box item, initially `on`, and makes the popup a check-box popup (flags 1, 4). */
DosItem *popup_add_check(DosPopup *p, char *s, i16 key, i16 on)
{
    DosItem *it;

    p->flags |= 5;
    it = popup_add_item(p, s, key);
    if (it != 0)
        it->value = on;
    return it;
}

/* Sets the text width; the box still grows to fit wider items or entry fields. */
void popup_set_width(DosPopup *p, i16 w)
{
    p->width = w;
}

/* Appends a text line. "^^text" stands on its own line, centred; "^text" on its own line; any
 * other line joins the paragraph, which is word-wrapped.
 */
DosLine *popup_add_string(DosPopup *p, char *s)
{
    DosLine *prev;
    DosLine *item;

    item = p->lines;
    prev = 0;
    while (item != 0) {
        prev = item;
        item = item->next;
    }
    item = heap_get((i32)sizeof(DosLine), p->arena);   /* Port: sizeof */
    if (prev != 0)
        prev->next = item;
    else
        p->lines = item;
    item->next = 0;
    item->flags = 0;
    if (*s == '^') {
        s++;
        if (*s == '^') {
            item->flags = 1;
            s++;
        } else
            item->flags |= 2;
    }
    item->text = heap_get((u32)(msc__fstrlen(s) + 1), p->arena);
    msc__fstrcpy(item->text, s);
    p->n_lines++;
    return item;
}

/* Appends an entry field: the label (a space is added) and a field of len characters, each as
 * wide as a W, holding init cut to len. Initial text is marked (80h) so that the first key typed
 * replaces it.
 */
DosEdit *popup_add_entry(DosPopup *p, char *label, char *init, i16 len)
{
    i16 u;
    DosEdit *ed;
    DosEdit *prev;

    ed = p->edits;
    prev = 0;
    while (ed != 0) {
        prev = ed;
        ed = ed->next;
    }
    ed = heap_get((i32)sizeof(DosEdit), p->arena);   /* Port: sizeof */
    if (prev != 0)
        prev->next = ed;
    else
        p->edits = ed;
    ed->next = 0;
    ed->flags = 0;
    ed->label = heap_get((u32)(msc__fstrlen(label) + 2), p->arena);
    msc__fstrcpy(ed->label, label);
    msc__fstrcat(ed->label, " ");
    ed->label_w = style_text_width(&p->style, ed->label);
    ed->len = len;
    ed->buf = heap_get((i32)(len + 1), p->arena);
    ed->field_w = style_text_width(&p->style, "W") * len;
    p->edit_w = max(ed->label_w + ed->field_w + 10, p->edit_w);
    if (init == 0)
        ed->buf[0] = 0;
    else {
        msc__fstrncpy(ed->buf, init, len);
        ed->buf[len] = 0;
        if (msc__fstrlen(ed->buf) != 0)
            ed->flags |= 0x80;
    }
    p->n_edits++;
    return ed;
}

/* Returns the height of a row of text in font c: a 6-pixel font counts as 5, except while a
 * frameless popup runs (orig_flag).
 */
i16 font_height(DosFont *c)
{
    i16 r;

    r = c->height;
    if (r == 6 && orig_flag == 0)
        r = 5;
    return r;
}

/* Appends a row to the sprite column and returns it: icon idx of sheet (in a unit picker, unit
 * idx as a 16x16 piece) with an optional label, `key` being what popup_exec returns for it. The
 * rows stack down the column: a sprite's x is its row's top and x2 the next row's, as offsets
 * from the column's top, and a row is as tall as its icon or its label. spr_h keeps the widest
 * icon and spr_lw the widest label. A labelled sprite in a popup without items sets the text
 * width to 0.
 */
DosSprite *popup_add_sprite(DosPopup *p, SpriteSeries *sheet, i16 idx, char *label,
                            i16 key)
{
    i16 a;
    DosSprite *last;
    DosSprite *spr;
    i16 h;
    i16 x1;
    i16 x2;
    i16 len;

    spr = p->sprites;
    last = 0;
    while (spr != 0) {
        last = spr;
        spr = spr->next;
    }
    spr = heap_get((i32)sizeof(DosSprite), p->arena);   /* Port: sizeof */
    if (last != 0) {
        last->next = spr;
        spr->x = last->x2;
    } else {
        p->first_sprite = p->sprites = spr;
        spr->x = p->f46 + p->f4a;
    }
    spr->next = 0;
    spr->sheet = sheet;
    spr->idx = idx;
    spr->key = key;
    if (label != 0) {
        if (p->items == 0)
            p->width = 0;
        spr->label = heap_get((u32)(msc__fstrlen(label) + 1), p->arena);
        msc__fstrcpy(spr->label, label);
        len = style_text_width(&p->style, label) + p->f46 + p->f32;
        h = font_height(p->style.font);
    } else {
        spr->label = 0;
        len = 0;
        h = 0;
    }
    if (p->flags & 2)
        x1 = a = 0x10;
    else {
        a = sheet->icon[idx - 1].w;
        x1 = sheet->icon[idx - 1].h;
    }
    spr->x2 = max(x1, h) + p->f46 + spr->x;
    p->spr_h = max(p->spr_h, a);
    p->spr_lw = max(p->spr_lw, len);
    p->n_sprites++;
    return spr;
}

/* Attaches a cursor record (a, b) to the popup; only then does the active sprite get its box. */
DosAttach *popup_add_cursor(DosPopup *p, void *a, i16 b)
{
    /* Port: the record's sizeof, but at least the 20 bytes the original allocated */
    p->attach = heap_get((i32)sizeof(DosAttach) > 0x14 ? (i32)sizeof(DosAttach) : 0x14L, p->arena);
    p->attach->a = a;
    p->attach->b = b;
    return p->attach;
}

/* Draws a text line at (x, y), x from the left edge of the text; returns the x after it. */
i16 popup_draw_line(i16 x, i16 y, DosPopup *p, char *s)
{
    i16 r;

    return style_draw_text(x + (p->f48 + p->f2a), y, 0, &p->style, s);
}

/* Lays the text lines out in the popup's text width, word by word, drawing them when `draw` is
 * set, and returns the height they take. Plain lines run together into one paragraph; a ^ or ^^
 * line ends it and is drawn whole (^^ centred). A word that does not fit starts a new line, even
 * when the line is empty: a word wider than the box leaves an empty line above it.
 */
i16 popup_wrap_lines(i16 draw, DosPopup *p)
{
    char *s;
    i16 curws;
    i16 maxwx;
    char bufx[0x100];
    DosLine *line;
    i16 y;
    char *sp9;
    char word[0x50];
    i16 xv;
    i16 hv;
    i16 ww;
    i16 len;

    line = p->lines;
    maxwx = p->width - p->f48 * 2;
    y = p->f2c;
    bufx[0] = 0;
    curws = hv = 0;
    while (line != 0) {
        s = line->text;
        if (line->flags & 3) {
            if (bufx[0] != 0) {
                if (draw)
                    popup_draw_line(0, y, p, bufx);
                hv += font_height(p->style.font) + 1;
                y += font_height(p->style.font) + 1;
                bufx[0] = 0;
                curws = 0;
            }
            if (draw) {
                if (line->flags & 1)
                    xv = (maxwx >> 1) - (style_text_width(&p->style, s) >> 1);
                else
                    xv = 0;
                popup_draw_line(xv, y, p, s);
            }
            while (*s != 0)
                s++;
            hv += font_height(p->style.font) + 1;
            y += font_height(p->style.font) + 1;
        }
        while (msc__fstrlen(s) != 0) {
            while (*s == ' ')
                s++;
            sp9 = msc__fstrchr(s, ' ');
            if (sp9 != 0)
                *sp9 = 0;
            len = msc__fstrlen(s);
            word[0] = 0;
            if (bufx[0] != 0)
                msc_strcat(word, " ");
            msc__fstrcat(word, s);
            ww = style_text_width(&p->style, word);
            if (p->style.spacing + ww + curws > maxwx) {
                if (draw)
                    popup_draw_line(0, y, p, bufx);
                hv += font_height(p->style.font) + 1;
                y += font_height(p->style.font) + 1;
                while (word[0] == ' ')
                    msc_strcpy(word, word + 1);
                bufx[0] = 0;
                curws = 0;
            }
            msc__fstrcat(bufx, word);
            curws += p->style.spacing + ww;
            if (sp9 != 0)
                *sp9 = ' ';
            s += len;
        }
        line = line->next;
    }
    if (bufx[0] != 0) {
        if (draw)
            popup_draw_line(0, y, p, bufx);
        hv += font_height(p->style.font) + 1;
        y += font_height(p->style.font) + 1;
        bufx[0] = 0;
    }
    return hv;
}

/* Sizes and places the popup: the box from the sprite column, the wrapped text, the items and
 * the entry fields, centred on the 320x200 screen unless a position was given, then pulled back
 * on screen (it reports an error if it still starts off it). r_x, r_y, r_w and r_h, the area
 * popup_exec saves and copies to the screen, take in the portrait. A chief or the King stands
 * left of the box (tribes 0, 3, 5 and 7 and the King) or right of it, centred vertically. A
 * messenger or foreign portrait stands above the box, which overlaps its bottom by the sheet's
 * tab[4] rows, at the box's left (tab[5] 0), centre (1) or right (2), overlapping by tab[6]
 * columns; one too tall for the screen is dropped (flag 40h). The portrait's position goes in
 * the dude's idx (x) and x (y). Finally the origins (f24/f26 items and fields, f2a/f2c text,
 * f36/f38) become screen coordinates. Returns 1 when the popup is empty, else 0.
 */
i16 popup_layout(DosPopup *p)
{
    i16 a14;
    i16 u1;
    i16 sxn;
    i16 u2s;
    i16 ih9;
    DosSprite *spr5;
    i16 tots;
    i16 a12v;
    i16 a10x;
    i16 ok;
    i16 u3s;
    i16 off;
    i16 th;
    i16 nhv;
    i16 tw1;
    i16 dh;
    i16 left1;
    i16 bottom7;
    i16 dw1;
    i16 eh_;
    i16 rightn;

    ok = 1;
    eh_ = ih9 = th = 0;
    if (p->n_edits != 0 && p->n_items != 0) {
        p->n_edits = 0;
        p->edits = 0;
    }
    p->x = p->pos_x;
    p->y = p->pos_y;
    p->w = 0;
    p->h = p->f4a * 2 + p->f46;
    p->f2a = (p->flags & 0x10) ? 0 : 3;
    p->f2c = p->f2a + p->f46;
    p->f24 = p->f2a;
    p->f26 = p->f2c;
    p->item_w = p->edit_w = p->width = max(max(p->width, p->item_w), p->edit_w);
    if (p->n_items + p->n_lines + p->n_sprites + p->n_edits == 0)
        goto out;
    {
        sxn = 0;
        if (p->sprites != 0) {
            u1 = p->f48 + p->spr_h + p->spr_lw + p->f32;
            p->w += u1;
            p->f2a += u1;
            p->f24 += u1;
            p->f36 += u1;
            spr5 = p->sprites;
            while (spr5 != 0) {
                sxn = spr5->x2;
                spr5 = spr5->next;
            }
        }
        if (p->lines != 0) {
            th = popup_wrap_lines(0, p);
            p->f26 += p->f46 + th;
            p->f38 += p->f46 + th;
        }
        if (p->items != 0)
            ih9 = (font_height(p->style.font) + p->f46) * p->n_items;
        if (p->edits != 0)
            eh_ = (font_height(p->style.font) + p->f46 + 5) * p->n_edits;
        tots = eh_ + ih9 + th;
        if (tots != 0)
            tots += p->f46;
        p->h += max(sxn, tots) + ((p->flags & 0x10) ? 0 : 3) * 2;
        p->w += p->item_w + ((p->flags & 0x10) ? 0 : 3) * 2;
        if (popup_help) {
            if (p->style.font == menu_font)
                p->h += 6;
            else
                p->h += 3;
        }
        if (p->x == -1)
            p->x = 160 - (p->w >> 1);
        if (p->y == -1)
            p->y = 100 - (p->h >> 1);
        bottom7 = p->h + p->y;
        rightn = p->w + p->x;
        if (rightn > 320)
            p->x += 320 - rightn;
        if (bottom7 > 200)
            p->y += 200 - bottom7;
        if (p->x < 0 || p->y < 0)
            error_report(-0x51, 2, 0x29, (i32)p->x, (i32)p->y);
        p->r_x = p->x;
        p->r_y = p->y;
        p->r_w = p->w;
        p->r_h = p->h;
        if (p->dude != 0) {
            spr5 = p->dude;
            if (popup_indian >= 0) {
                dh = spr5->sheet->icon[0].h + 3;
                off = -3;
                dw1 = spr5->sheet->icon[0].w + 3;
                tw1 = p->w + dw1 + 3;
                if (tw1 > 320) {
                    off = tw1 - 0x143;
                    tw1 = 320;
                }
                if (popup_indian == 0 || popup_indian == 3 || popup_indian == 5
                    || popup_indian == 7 || popup_indian == 8)
                    left1 = 1;
                else
                    left1 = 0;
                p->r_w = tw1;
                if (left1) {
                    p->r_x = 160 - (tw1 >> 1);
                    spr5->idx = p->r_x;
                    p->x = p->r_x - off + dw1;
                } else {
                    p->x = p->r_x = 160 - (tw1 >> 1);
                    spr5->idx = p->x + p->w - off;
                }
                spr5->x = 100 - (dh >> 1);
                p->r_y = min(spr5->x, p->y);
                p->r_h = max(spr5->x + dh - 1, bottom7) - p->r_y + 1;
            }
            if (popup_messenger < 0 && popup_foreign < 0)
                goto done;
            dw1 = spr5->sheet->icon[0].w;
            dh = spr5->sheet->icon[0].h;
            a10x = spr5->sheet->tab[4];
            a12v = spr5->sheet->tab[5];
            a14 = spr5->sheet->tab[6];
            nhv = dh - min(a10x, dh) + p->h;
            if (nhv >= 200) {
                p->flags |= 0x40;
                goto done;
            }
            spr5->x = 100 - (nhv >> 1);
            p->r_y = spr5->x;
            p->r_h = max(nhv, dh);
            p->y = spr5->x - a10x + dh;
            switch (a12v) {
            case 0:
                tw1 = p->w - a14 + dw1;
                if (tw1 > 320) {
                    a14 += tw1 - 320;
                    tw1 = 320;
                }
                spr5->idx = 160 - (tw1 >> 1);
                p->r_x = spr5->idx;
                p->r_w = tw1;
                p->x = spr5->idx - a14 + dw1;
                break;
            case 1:
                u3s = p->x;
                spr5->idx = 160 - (dw1 >> 1);
                p->r_x = min(u3s, spr5->idx);
                rightn = max(spr5->idx + dw1 - 1, rightn);
                p->r_w = rightn - p->r_x + 1;
                break;
            case 2:
                tw1 = p->w - a14 + dw1;
                if (tw1 > 320) {
                    a14 += tw1 - 320;
                    tw1 = 320;
                }
                p->r_x = p->x = 160 - (tw1 >> 1);
                p->r_w = tw1;
                spr5->idx = p->x + p->w - a14;
                break;
            }
        }
    done:
        p->f24 += p->x;
        p->f26 += p->y;
        p->f2a += p->x;
        p->f2c += p->y;
        p->f36 += p->x;
        p->f38 += p->y;
        ok = 0;
    }
out:
    return ok;
}

/* Copies the popup's area from scr_work to the screen, unless popup_no_draw is set and the popup
 * is draw-only (20h).
 */
void popup_show(DosPopup *p)
{
    if (popup_no_draw == 0 || (p->flags & 0x20) == 0)
        mouse_video_update(p->r_x, p->r_y, p->r_x, p->r_y, p->r_w, p->r_h);
}

/* When popup_help is set, draws the help word right-aligned in the box's bottom-right corner.
 * Its string number is spot[10].v2, the word of the hot-spot table the original reads there.
 */
void popup_help_draw(DosPopup *p)
{
    i16 x;
    i16 yv;
    char buf[0x50];

    if (popup_help) {
        x = p->w + p->x - 2;
        yv = p->h + p->y - 7;
        if (p->style.font == menu_font)
            yv -= 2;
        buf[0] = 0;
        say_string(buf, spot[10].v2);
        write_right(buf, x, yv, hilite_color);
    }
}

/* Draws the portrait, unless it did not fit (40h), and over it the King's current frame from his
 * second sheet once he animates.
 */
void popup_big_sprite(DosPopup *p)
{
    DosSprite *d;
    i16 king;
    u16 vx;
    i16 wx;
    i16 uy;

    king = popup_indian > 7 ? 1 : 0;
    if (p->dude != 0 && (p->flags & 0x40) == 0) {
        d = p->dude;
        sprite_draw(vx = 1, d->sheet, &scr_work, d->idx, d->x);
        if (king && king_active && king_frame > 0)
            sprite_draw(wx = king_frame, d->next->sheet, &scr_work, d->idx, d->x);
    }
}

/* Draws the items: each row's text, the selected row on a bar, a line for a separator, the
 * check-box state, and the text after | right-aligned. With `full` it first refills the item
 * column's background, and afterwards draws a messenger or foreign portrait (which overlaps the
 * box) and the help word and copies the popup to the screen.
 */
void popup_draw_items(i16 full, DosPopup *p)
{
    i16 bwx;
    char *bar;
    i16 x;
    i16 y;
    i16 cy;
    DosItem *it;

    x = p->f24 + p->f48 + p->list_rows;
    y = p->f26;
    it = p->items;
    if (full)
        popup_fill(x - p->list_rows - 1, y, (1 - p->f48) * 2 + p->item_w, scr_work,
                   (font_height(p->style.font) + p->f46) * p->n_items, p->x, p->y, p->w,
                   (char)p->col0, (char)p->col1, 0, 0);
    while (it != 0) {
        popup_hiliting = it->flags & 2;
        if (it == p->sel_item)
            popup_fill(x - p->list_rows - 1, y, (1 - p->f48) * 2 + p->item_w, scr_work,
                       font_height(p->style.font) + 2, p->x, p->y, p->w,
                       (char)p->col2, (char)p->col3, 0, 0);
        if (*it->text == 0) {
            cy = (font_height(p->style.font) >> 1) + y;
            buffer_rect_fill(x - p->list_rows, cy, p->item_w - p->f48 * 2, scr_work, 1,
                      (char)p->style.color);
        } else {
            if (p->flags & 4)
                it->text[1] = it->value == 0 ? '[' : ']';
            style_draw_text(x, y + 1, it->flags, &p->style, it->text);
            bar = msc__fstrchr(it->text, '|');
            if (bar != 0) {
                bar++;
                bwx = style_text_width(&p->style, bar);
                style_draw_text(p->item_w - (p->f48 + p->list_rows) * 2 - bwx + x, y + 1,
                                it->flags, &p->style, bar);
            }
        }
        y += font_height(p->style.font) + p->f46;
        it = it->next;
    }
    if (full) {
        if (popup_indian < 0)
            popup_big_sprite(p);
        popup_help_draw(p);
        popup_show(p);
    }
}

/* Draws the entry fields: the label, a box, and the text with a _ cursor while there is room;
 * initial (untyped) text sits on the selection colour. With `full` it also draws the help word
 * and copies the popup to the screen.
 */
void popup_draw_edits(i16 full, DosPopup *p)
{
    i16 fxs;
    i16 ix;
    i16 fys;
    i16 iy;
    DosEdit *ed;
    char buf[0x50];
    i16 iwn;
    i16 tyb;
    i16 yv;
    i16 x7;
    i16 fh1;
    i16 ihn;
    i16 fw2;

    popup_hiliting = 0;
    x7 = p->f24 + p->f48;
    yv = p->f26;
    ed = p->edits;
    while (ed != 0) {
        msc__fstrcpy(buf, ed->buf);
        if ((i16)msc_strlen(buf) < ed->len)
            msc_strcat(buf, "_");
        tyb = yv + 3;
        style_draw_text(x7, tyb, 0, &p->style, ed->label);
        fxs = ed->label_w + x7;
        fys = yv;
        fw2 = ed->field_w + 6;
        fh1 = font_height(p->style.font) + 5;
        buffer_draw_box(fxs, yv, fw2 + fxs - 1, scr_work, fh1 + yv - 1, (char)p->style.color);
        ix = fxs + 2;
        iwn = fw2 - 4;
        iy = yv + 2;
        ihn = fh1 - 4;
        popup_fill(ix, iy, iwn, scr_work, ihn, p->x, p->y, p->w, (char)p->col0,
                   (char)p->col1, 0, 0);
        if (ed->flags & 0x80) {
            iwn = style_text_width(&p->style, ed->buf) + 2;
            popup_fill(ix, iy, iwn, scr_work, ihn, p->x, p->y, p->w, (char)p->col2,
                       (char)p->col3, 0, 0);
        }
        style_draw_text(fxs + 3, fys + 3, 0, &p->style, buf);
        ed = ed->next;
    }
    if (full) {
        popup_help_draw(p);
        popup_show(p);
    }
}

/* Draws the sprite column: each icon (a unit piece in a unit picker) and, with `labels`, its
 * label beside it. With `hilite`, the active sprite gets a white box in keyboard mode when a
 * cursor is attached. With `full` it first refills the background behind the icons, and
 * afterwards draws the help word and copies the popup to the screen.
 */
void popup_draw_sprites(i16 labels, i16 full, i16 hilite, DosPopup *p)
{
    i16 oyn;
    i16 ua;
    i16 ub;
    i16 ox;
    i16 uc;
    i16 lx4;
    DosSprite *sprs;
    i16 ud;
    i16 ihx;
    i16 lastx;

    popup_hiliting = 0;
    if (p->sprites == 0)
        return;
    sprs = p->sprites;
    while (sprs->next != 0)
        sprs = sprs->next;
    lastx = sprs->x2;
    sprs = p->sprites;
    ub = ((p->flags & 0x10) ? 0 : 3) + p->x;
    oyn = ((p->flags & 0x10) ? 0 : 3) + p->y;
    ox = ub + p->f48;
    ua = oyn + p->f46;
    if (p->flags & 2)
        ihx = 0x10;
    else
        ihx = sprs->sheet->icon[sprs->idx - 1].w;
    uc = lastx - sprs->x + 1;
    if (full)
        popup_fill(ox - 1, sprs->x + oyn - 1, ihx + 2, scr_work, uc + 2,
                   p->x, p->y, p->w, (char)p->col0, (char)p->col1, 0, 0);
    while (sprs != 0) {
        if (p->flags & 2)
            piece_draw(sprs->idx, 0, ox, sprs->x + oyn, 0x10, 0x64);
        else
            sprite_draw(sprs->idx, sprs->sheet, &scr_work, ox, sprs->x + oyn);
        if (sprs == p->first_sprite && hilite && (p->flags & 0x80) && p->attach != 0)
            buffer_draw_box(ox - 1, sprs->x + oyn - 1, ox + 0x10, scr_work, sprs->x + oyn + 0x10, 0xf);
        if (labels && sprs->label != 0) {
            lx4 = p->f46 + p->spr_h + p->f32 + ox;
            ud = ((sprs->x2 - sprs->x) >> 1) - (font_height(p->style.font) >> 1) + sprs->x + oyn;
            style_draw_text(lx4, ud, 0, &p->style, sprs->label);
        }
        sprs = sprs->next;
    }
    if (full) {
        popup_help_draw(p);
        popup_show(p);
    }
}

/* Draws a box at (x, y, w, h) in scr_work: unless p is frameless, a 3-pixel frame (black, then
 * popup_border_color, then a bevel lit at the top and right and shaded at the left and bottom),
 * then the background inside it, tiled from p's corner (the box's own when p is 0).
 */
void popup_box_draw(DosPopup *p, i16 x, i16 y, i16 w, i16 h)
{
    i16 ix;
    i16 iy;
    i16 iws;
    i16 ihs;
    i16 px;
    i16 py;
    i16 pw2;
    i16 havepx;

    havepx = 0;
    if (p != 0)
        havepx = 1;
    if (havepx) {
        px = p->x;
        py = p->y;
        pw2 = p->w;
    } else {
        px = x;
        py = y;
        pw2 = w;
    }
    if (p == 0 || (p->flags & 0x10) == 0) {
        buffer_draw_box(x, y, w + x - 1, scr_work, h + y - 1, 0);
        buffer_draw_box(x + 1, y + 1, w + x - 2, scr_work, h + y - 2, (char)popup_border_color);
        buffer_vline(x + 2, y + 2, h + y - 3, scr_work, (char)popup_border_color_down);
        buffer_vline(w + x - 3, y + 2, h + y - 3, scr_work, (char)popup_border_color_up);
        buffer_hline(x + 2, w + x - 3, y + 2, scr_work, (char)popup_border_color_up);
        buffer_hline(x + 2, w + x - 3, h + y - 3, scr_work, (char)popup_border_color_down);
    }
    ix = (havepx ? ((p->flags & 0x10) ? 0 : 3) : 3) + x;
    iy = (havepx ? ((p->flags & 0x10) ? 0 : 3) : 3) + y;
    iws = w - (havepx ? ((p->flags & 0x10) ? 0 : 3) : 3) * 2;
    ihs = h - (havepx ? ((p->flags & 0x10) ? 0 : 3) : 3) * 2;
    popup_fill(ix, iy, iws, scr_work, ihs, px, py, pw2, (char)popup_background_color_0,
               (char)popup_background_color_1, 0, 0);
}

/* Lays the popup out and draws all of it: a chief's portrait behind the box, the box, the sprite
 * column, the text, the items, the entry fields, any other portrait over the box and the help
 * word, then copies it to the screen. Returns 1, drawing nothing, when the popup is empty.
 */
i16 popup_draw(DosPopup *p)
{
    char bufs[0x54];
    i16 r;
    i16 x;
    i16 y;
    i16 ws;
    i16 hs;

    r = 1;
    if (popup_layout(p) == 0) {
        popup_hiliting = 0;
        x = p->x;
        y = p->y;
        ws = p->w;
        hs = p->h;
        if (popup_indian >= 0)
            popup_big_sprite(p);
        popup_box_draw(p, x, y, ws, hs);
        popup_draw_sprites(1, 0, 1, p);
        popup_hiliting = 0;
        popup_wrap_lines(1, p);
        popup_draw_items(0, p);
        popup_draw_edits(0, p);
        if (popup_indian < 0)
            popup_big_sprite(p);
        popup_help_draw(p);
        popup_show(p);
        r = 0;
    }
    return r;
}

/* Makes spr the active sprite (0: none) and redraws the sprite column. */
void popup_set_active_sprite(DosPopup *p, DosSprite *spr)
{
    p->first_sprite = spr;
    popup_draw_sprites(0, 1, 1, p);
}

/* Runs popup p and returns its result: the chosen item's or sprite's key, 0 when it closes with
 * nothing chosen, -1 for Escape. It loads the portrait, lays the popup out, saves what it covers
 * (unless flag 08h), draws it and loops on the mouse and the keyboard:
 *   items    Up and Down move the selection, wrapping and skipping greyed items, and a hotkey
 *            selects its item; Space, Enter and F1 choose the selected item, except that in a
 *            check-box popup Space, F1 and a hotkey toggle it and Enter closes. Dragging over
 *            an item selects it and releasing chooses (or toggles) it; dragging outside the box
 *            deselects, and releasing there closes only a flag-01h popup. A release inside the
 *            box but between items does nothing.
 *   sprites  Up and Down move the active sprite; any other key, or a release, closes with it
 *            (none, outside the box). In a unit picker, closing on a unit with orders clears its
 *            orders instead and stays open.
 *   fields   printable keys type (the first replaces the initial text), Backspace deletes,
 *            Delete clears the initial text, Enter or a click closes. The text goes to
 *            popup_text_buffer, even after Escape.
 * F1 with popup_help, or closing with the right button with popup_help, sets popup_help_me. In
 * autoplay (automate) the popup closes itself after 120 ticks. The King's portrait steps a frame
 * every 10 ticks. On the way out it restores the screen, frees the portrait sheets, stores the
 * check-box states in popup_checkbox, resets the portrait selectors and popup_help, and reloads
 * the main sprites if a portrait freed them. An empty popup returns 0 without drawing; a
 * draw-only one (20h) is drawn and returned from at once. Both skip freeing the portrait sheets.
 */
i16 popup_exec(DosPopup *p)
{
    i16 redraw;
    i16 i;
    i16 king;
    DosSprite *dude7;
    char chn[2];
    i16 savedv;
    DosSprite *spr4;
    i16 iy7;
    i32 t05;
    i16 fullv;
    i16 runningx;
    i16 rowh9;
    DosEdit *ed;
    i16 key4;
    i32 now9;
    DosItem *it;
    i16 outside;
    i16 triesv;
    i16 ux;
    i16 found1;
    i16 oldn;
    i16 n;

    fullv = 1;
    king = popup_indian > 7 ? 1 : 0;
    popup_help_me = redraw = 0;
    if (p->flags & 0x10)
        orig_flag = 1;
    else
        orig_flag = 0;
    popup_hiliting = 0;
    text_close();
    while (keys_any())
        keys_get();
    if (p->flags & 4)
        for (i = 0; i < p->n_items; i++)
            popup_write_check(p, i + 1, popup_checkbox & (1 << i));
    p->f0 = 0;
    rowh9 = font_height(p->style.font) + p->f46;
    if (popup_indian >= 0)
        popup_dude_tribe(p);
    if (popup_messenger >= 0)
        popup_dude_ms(p);
    if (popup_foreign >= 0)
        popup_dude_myr(p);
    popup_load_dude(p);
    if (p->dude != 0) {
        spr4 = p->dude;
        dude7 = spr4->next;
    }
    if (popup_layout(p) != 0)
        goto done;
    if (king_active != 0)
        buffer_rect_copy(0, 0, 0x140, scr_work, scr_orig, 200);
    /* Port: the original passed mode and src as one long, -8L; here, -8 and -1. */
    if ((p->flags & 8) == 0)
        savedv = buffer_preserve(&scr_work, -8, -1, p->r_x, p->r_y, p->r_w, p->r_h);
    runningx = 1;
    t05 = timer_read();
    mouse_init_cycle();
    if (popup_pal) {
        oldn = cycling_active;
        cycling_active = 0;
        mcga_setpal((char *)(msc_vga + 0xfc00));
        cycling_active = oldn;
    }
    if (p->flags & 0x20) {
        popup_draw(p);
        goto done;
    }
    do {
        check_music();
        mouse_begin_cycle(0);
        outside = 0;
        if (mouse_any_stroke && mouse_changed) {
            if (p->x > mouse_x || p->y > mouse_y || p->w + p->x <= mouse_x
                || p->h + p->y <= mouse_y) {
                redraw = 1;
                p->sel_item = 0;
                popup_set_active_sprite(p, 0);
            } else if (p->items != 0) {
                it = p->items;
                iy7 = p->f26;
                found1 = 0;
                do {
                    if (it == 0)
                        break;
                    if (iy7 - 1 <= mouse_y && rowh9 + iy7 - 1 > mouse_y && (it->flags & 1) == 0) {
                        p->sel_item = it;
                        redraw = found1 = 1;
                    }
                    iy7 += rowh9;
                    it = it->next;
                } while (found1 == 0);
                if (found1 == 0)
                    outside = 1;
            } else if (p->sprites != 0) {
                spr4 = p->sprites;
                found1 = 0;
                do {
                    if (spr4 == 0)
                        break;
                    if (((p->flags & 0x10) ? 0 : 3) + p->y + spr4->x <= mouse_y
                        && ((p->flags & 0x10) ? 0 : 3) + spr4->x2 + p->y > mouse_y) {
                        found1 = 1;
                        popup_set_active_sprite(p, spr4);
                    }
                    spr4 = spr4->next;
                } while (found1 == 0);
                if (found1 == 0)
                    outside = 1;
            }
        }
        if (runningx && keys_any()) {
            key4 = keys_get();
            if (p->edits != 0) {
                ed = p->edits;
                switch (key4) {
                case 0x13b:
                    if (popup_help) {
                        runningx = 0;
                        popup_help_me = 1;
                    }
                    break;
                default:
                    if (key4 < 0x100 && isprint(key4)) {
                        if (ed->flags & 0x80) {
                            *ed->buf = 0;
                            ed->flags &= ~0x80;
                        }
                        chn[0] = key4;
                        chn[1] = 0;
                        if (msc__fstrlen(ed->buf) < ed->len)
                            msc__fstrcat(ed->buf, chn);
                        popup_draw_edits(1, p);
                    }
                    break;
                case 8:
                    if (msc__fstrlen(ed->buf) != 0) {
                        n = msc__fstrlen(ed->buf);
                        ed->buf[n - 1] = 0;
                    }
                    ed->flags &= ~0x80;
                    popup_draw_edits(1, p);
                    break;
                case 0x153:
                    if (ed->flags & 0x80) {
                        *ed->buf = 0;
                        ed->flags &= ~0x80;
                        popup_draw_edits(1, p);
                    }
                    break;
                case 0xd:
                    runningx = 0;
                    break;
                case 0x1b:
                    p->f0 = -1;
                    runningx = 0;
                    break;
                }
            } else {
                if (key4 < 0x100 && islower(key4))
                    key4 -= 0x20;
                if (p->items == 0) {
                    switch (key4) {
                    case 0x150:
                        p->flags |= 0x80;
                        if (p->sprites != 0) {
                            if (p->first_sprite == 0)
                                spr4 = p->sprites;
                            else {
                                spr4 = p->first_sprite->next;
                                if (spr4 == 0)
                                    spr4 = p->sprites;
                            }
                            popup_set_active_sprite(p, spr4);
                        }
                        break;
                    case 0x148:
                        p->flags |= 0x80;
                        if (p->sprites != 0) {
                            if (p->first_sprite == 0 || p->first_sprite == p->sprites) {
                                spr4 = p->sprites;
                                while (spr4->next != 0)
                                    spr4 = spr4->next;
                            } else {
                                spr4 = p->sprites;
                                while (spr4->next != p->first_sprite)
                                    spr4 = spr4->next;
                            }
                            popup_set_active_sprite(p, spr4);
                        }
                        break;
                    default:
                        p->flags |= 0x80;
                        if (p->first_sprite != 0 && (p->flags & 2)
                            && unit[ux = p->first_sprite->idx].orders != 0) {
                            unit[ux].orders = 0;
                            popup_set_active_sprite(p, p->first_sprite);
                        } else
                            runningx = 0;
                        break;
                    case 0x1b:
                        p->f0 = -1;
                        runningx = 0;
                        break;
                    }
                } else {
                    switch (key4) {
                    case 0x1b:
                        p->f0 = -1;
                        runningx = 0;
                        break;
                    case 0x150:
                        triesv = 0;
                        do {
                            if (p->sel_item != 0)
                                p->sel_item = p->sel_item->next;
                            if (p->sel_item == 0) {
                                p->sel_item = p->items;
                                triesv++;
                            }
                            if ((p->sel_item->flags & 1) == 0)
                                break;
                        } while (triesv < 2);
                        goto set_redraw;
                    case 0x148:
                        triesv = 0;
                        do {
                            if (p->sel_item != 0)
                                p->sel_item = p->sel_item->prev;
                            if (p->sel_item == 0) {
                                p->sel_item = (DosItem *)p->f70;
                                triesv++;
                            }
                            if ((p->sel_item->flags & 1) == 0)
                                break;
                        } while (triesv < 2);
                        goto set_redraw;
                    case ' ':
                    case 0xd:
                    case 0x13b:
                        if (p->sel_item == 0)
                            break;
                        if (p->flags & 4) {
                            if (key4 != 0xd)
                                p->sel_item->value = p->sel_item->value == 0;
                            if (key4 == 0xd)
                                runningx = 0;
                        } else {
                            if (key4 == 0x13b && popup_help == 0)
                                break;
                            p->f0 = p->sel_item->key;
                            runningx = 0;
                            if (key4 == 0x13b) {
                                popup_help_me = 1;
                                break;
                            }
                            break;
                        }
                    set_redraw:
                        redraw = 1;
                        break;
                    default:
                        found1 = 0;
                        it = p->items;
                        while (found1 == 0) {
                            if (it == 0)
                                goto hot_end;
                            if (it->hotkey == key4)
                                found1 = 1;
                            else
                                it = it->next;
                        }
                    hot_end:
                        if (found1 == 0)
                            break;
                        p->sel_item = it;
                        redraw = 1;
                        if (p->flags & 4)
                            it->value = it->value == 0;
                        else
                            runningx = 0;
                        break;
                    }
                }
            }
        }
        /* dude7 is set only when a portrait loaded; if the King's failed, the original
         * reads it uninitialised here.
         */
        if (king_active && dude7 != 0 && timer_read() >= king_clock
            && dude7->sheet->count > king_frame) {
            king_frame++;
            buffer_rect_copy(0, 0, 0x140, scr_orig, scr_work, 200);
            fullv = 1;
            king_clock = timer_read() + 10;
        }
        if (fullv)
            popup_draw(p);
        else if (redraw)
            popup_draw_items(1, p);
        redraw = fullv = 0;
        if (mouse_stop_stroke != 0 && outside == 0) {
            if (p->items != 0) {
                if (p->sel_item != 0 || (p->flags & 1)) {
                    if (p->sel_item == 0)
                        runningx = 0;
                    else if (p->flags & 4) {
                        p->sel_item->value = p->sel_item->value == 0;
                        redraw = 1;
                    } else
                        runningx = 0;
                }
            } else if (p->sprites != 0 && p->first_sprite != 0 && (p->flags & 2)
                       && unit[ux = p->first_sprite->idx].orders != 0) {
                unit[ux].orders = 0;
                popup_set_active_sprite(p, p->first_sprite);
            } else
                runningx = 0;
        }
        if (mouse_stop_stroke && runningx == 0 && popup_help && mouse_button)
            popup_help_me = 1;
        if (runningx && mouse_stroke_going)
            n = 1;
        else
            n = 0;
        mouse_end_cycle(0, n);
        if (automate) {
            now9 = timer_read();
            if (now9 >= t05 + 120)
                runningx = 0;
        }
    } while (runningx);
    if (p->edits != 0)
        msc__fstrcpy(popup_text_buffer, p->edits->buf);
    else if (p->f0 == 0) {
        if (p->sel_item != 0)
            p->f0 = p->sel_item->key;
        else if (p->first_sprite != 0)
            p->f0 = p->first_sprite->key;
    }
    if ((p->flags & 8) == 0)
        buffer_restore(&scr_work, savedv, -1, p->r_x, p->r_y, p->r_w, p->r_h);
    if (p->dude != 0) {
        if (sprite_tight != 1)
            mem_free(p->dude->sheet);
        if (king && sprite_tight == 0) {
            spr4 = p->dude->next;
            mem_free(spr4->sheet);
        }
    }
    popup_show(p);
done:
    popup_foreign = popup_messenger = popup_indian = -1;
    popup_help = orig_flag = 0;
    if (p->flags & 4) {
        popup_checkbox = 0;
        for (i = 0; i < p->n_items; i++)
            if (popup_read_check(p, i + 1))
                popup_checkbox |= 1 << i;
    }
    mouse_init_cycle();
    if (sprite_tight)
        load_main_sprites();
    return p->f0;
}

DosFont *popup_font = 0;                /* font for popup_start_box */
i16 popup_heap = 0x0c00;                /* arena size for popup_start_box */

/* Sets the font and the arena size popup_start_box creates popups with. */
void popup_init(DosFont *font, i16 size)
{
    popup_font = font;
    popup_heap = size;
}

/* Copies src to dst, expanding %STRINGn (popup_subst_string), %NUMBERn (popup_subst_number in
 * decimal), %HEXn (in hexadecimal, at least four digits), %COUNTRY (the player's country, empty
 * in the editor), %YEAR (game.year) and %%; n is one digit. A % before anything else is dropped.
 * STRING is matched in any case, the other names in capitals only. src is modified: each % in it
 * is overwritten with a 0.
 */
void popup_parse_string(char *src, char *dst)
{
    char tmp[0x28];
    i16 i;
    char *mark;
    i16 n;

    *dst = 0;
    do {
        mark = msc_strchr(src, '%');
        if (mark != 0)
            *mark = 0;
        if (*src != 0)
            msc_strcat(dst, src);
        if (mark != 0) {
            mark++;
            src = mark;
            if (msc_strnicmp(mark, "STRING", 6) == 0) {
                n = msc_atoi(mark + 6);
                msc_strcat(dst, popup_subst_string[n]);
                src += 7;
            } else if (msc_strncmp(mark, "NUMBER", 6) == 0) {
                n = msc_atoi(mark + 6);
                msc_ltoa(popup_subst_number[n], tmp, 10);
                msc_strcat(dst, tmp);
                src += 7;
            } else if (msc_strncmp(mark, "HEX", 3) == 0) {
                n = msc_atoi(mark + 3);
                msc_ltoa(popup_subst_number[n], tmp, 16);
                /* With more than four digits, the 16-bit original's unsigned 4 - strlen wraps
                 * and this loop runs about 65,000 times, overrunning dst; in the port the
                 * difference is signed and no zero is added.
                 */
                for (i = 0; 4 - msc_strlen(tmp) > i; i++)
                    msc_strcat(dst, "0");
                msc_strcat(dst, tmp);
                src += 4;
            } else if (msc_strncmp(mark, "COUNTRY", 7) == 0) {
                tmp[0] = 0;
                say_country(game.player_nation, 0, tmp);
                msc__fstrcat(dst, tmp);
                src += 7;
            } else if (msc_strncmp(mark, "YEAR", 4) == 0) {
                msc_itoa(game.year, tmp, 10);
                msc__fstrcat(dst, tmp);
                src += 4;
            } else if (*mark == '%') {
                msc__fstrcat(dst, "%");
                src++;
            }
        }
    } while (mark != 0);
}

i16 popup_start_ask = 0;                /* popup_start_box builds an entry field */

/* Builds a popup from section @key of text file txt (.TXT is added) and returns it, or 0 when
 * the section or the memory is missing; then it also resets the portrait selectors and
 * popup_help. Text lines and options are expanded with popup_parse_string. The options become
 * items numbered from 1, item sel preselected, or, for popup_ask (popup_start_ask), entry fields
 * labelled by them, only the first of which takes keys. key is copied to error_string for error
 * reports.
 */
DosPopup * popup_start_box(char *txt, char *key, i16 sel)
{
    i16 w;
    i16 modes;
    DosEdit *ed;
    i16 itemno;
    char defbuf[0x50];
    char buf_[0x100];
    i16 checkbox5;
    char *line4;
    DosPopup *popupx;
    DosItem *it;
    char *pn;

    itemno = modes = 1;
    checkbox5 = 0;
    popupx = 0;
    msc_strcpy(error_string, key);
    if (text_open(txt, key) != 0)
        goto out;
    popupx = popup_create(popup_heap, popup_font);
    if (popupx == 0)
        goto out;
    if (popup_start_ask != 0 && popup_start_string != 0)
        msc_strcpy(defbuf, popup_start_string);
    do {
        line4 = text_get();
        if (msc_strlen(line4) == 0) {
            modes++;
            continue;
        }
        if (*line4 == '@') {
            msc_strupr(line4);
            pn = line4 + 1;
            if (msc_strcmp(pn, "OPTIONS") == 0 || msc_strcmp(pn, "PROMPT") == 0)
                modes = 2;
            else if (msc_strcmp(pn, "TEXT") == 0)
                modes = 1;
            else if (msc_strcmp(pn, "SMALLFONT") == 0)
                popupx->style.font = menu_font;
            else if (msc_strncmp(pn, "Y", 1) == 0) {
                while (*pn != 0) {
                    if (!isdigit(*pn)) {
                        pn++;
                        continue;
                    }
                    break;
                }
                popupx->pos_y = msc_atoi(pn);
            } else if (msc_strncmp(pn, "X", 1) == 0) {
                while (*pn != 0) {
                    if (!isdigit(*pn)) {
                        pn++;
                        continue;
                    }
                    break;
                }
                popupx->pos_x = msc_atoi(pn);
            } else if (msc_strncmp(pn, "WIDTH", 5) == 0) {
                while (*pn != 0) {
                    if (!isdigit(*pn)) {
                        pn++;
                        continue;
                    }
                    break;
                }
                w = msc_atoi(pn);
                popup_set_width(popupx, w);
            } else if (msc_strnicmp(pn, "LENGTH", 6) == 0) {
                while (*pn != 0) {
                    if (!isdigit(*pn)) {
                        pn++;
                        continue;
                    }
                    break;
                }
                if (popup_start_max_len == 0)
                    popup_start_max_len = msc_atoi(pn);
            } else if (msc_strncmp(pn, "CHECKBOX", 7) == 0) {
                checkbox5 = 1;
                popupx->flags |= 5;
            } else if (msc_strncmp(pn, "DEFAULT", 7) == 0) {
                if (popup_start_ask != 0) {
                    while (*pn != 0) {
                        if (*pn != '=') {
                            pn++;
                            continue;
                        }
                        break;
                    }
                    if (popup_start_string == 0) {
                        if (*pn != 0)
                            pn++;
                        msc_strcpy(defbuf, pn);
                    }
                } else {
                    while (*pn != 0) {
                        if (!isdigit(*pn)) {
                            pn++;
                            continue;
                        }
                        break;
                    }
                    if (sel == 0)
                        sel = msc_atoi(pn);
                }
            } else
                modes = 3;
        } else {
            switch (modes) {
            case 1:
                popup_parse_string(line4, buf_);
                popup_add_string(popupx, buf_);
                break;
            case 2:
                if (popup_start_ask != 0) {
                    if (popup_start_max_len == 0)
                        popup_start_max_len = 5;
                    popup_parse_string(line4, buf_);
                    ed = popup_add_entry(popupx, buf_, defbuf, popup_start_max_len);
                } else {
                    popup_parse_string(line4, buf_);
                    it = popup_add_item(popupx, buf_, itemno);
                    if (checkbox5)
                        it->value = 0;
                    if (itemno == sel)
                        popupx->sel_item = it;
                    itemno++;
                }
                break;
            }
        }
    } while (modes < 3);
out:
    if (popupx == 0) {
        popup_foreign = popup_indian = popup_messenger = -1;
        popup_help = 0;
    }
    return popupx;
}

/* Shows section key of text file `file` and returns popup_exec's result (0 when the section
 * cannot be built).
 */
i16 popup_box(char *file, char *key, i16 sel)
{
    i16 r;
    DosPopup *pop;

    r = 0;
    pop = popup_start_box(file, key, sel);
    if (pop != 0) {
        r = popup_exec(pop);
        mem_free(pop);
    }
    return r;
}

/* Clears the check-box states for the next check-box popup. */
void pop_clear(void)
{
    popup_checkbox = 0;
}

/* Sets check box `bit` (counted from 1) for the next check-box popup, or clears it. */
void pop_set(i16 bit, i16 on)
{
    bit--;
    if (on)
        popup_checkbox |= 1 << bit;
    else
        popup_checkbox &= ~(1 << bit);
}

/* Returns check box n's state (counted from 1) after a check-box popup: nonzero when checked. */
i16 pop_get(i16 n)
{
    n -= 1;
    return (1 << n) & popup_checkbox;
}

/* Shows section key of the game's text file. */
i16 pop(char *key)
{
    return popup_box(game_text_name, key, 0);
}

/* Shows section key of the game's text file with item sel preselected. */
i16 pop2(char *key, i16 sel)
{
    return popup_box(game_text_name, key, sel);
}

/* Shows section key of the game's text file with the chief of `tribe`. */
i16 popi(char *key, i16 tribe)
{
    popup_indian = tribe;
    return popup_box(game_text_name, key, 0);
}

/* pop2 with the chief of `tribe`. */
i16 pop2i(char *key, i16 sel, i16 tribe)
{
    popup_indian = tribe;
    return popup_box(game_text_name, key, sel);
}

/* Shows section key of the game's text file with the King. */
i16 popk(char *key)
{
    popup_indian = 8;
    return popup_box(game_text_name, key, 0);
}

/* Shows section key of the game's text file with messenger `dude`. */
i16 popm(char *key, i16 dude)
{
    popup_messenger = dude;
    return popup_box(game_text_name, key, 0);
}

/* pop2 with messenger `dude`. */
i16 pop2m(char *key, i16 sel, i16 dude)
{
    popup_messenger = dude;
    return popup_box(game_text_name, key, sel);
}

/* Shows section key of the game's text file with the envoy of `nation`. */
i16 popf(char *key, i16 nation)
{
    popup_foreign = nation;
    return popup_box(game_text_name, key, 0);
}

/* pop2 with the envoy of `nation`. */
i16 pop2f(char *key, i16 sel, i16 nation)
{
    popup_foreign = nation;
    return popup_box(game_text_name, key, sel);
}

/* Makes the next popup frameless (10h), leaving what it covers unsaved (08h). */
void popup_off(void)
{
    popup_flags |= 0x18;
}

/* Asks for a line of text: section key of txt with an entry field of up to max characters,
 * initially dflt. Returns popup_exec's result (0 for Enter or a click, -1 for Escape) or 0 when
 * the section cannot be built; the text is left in popup_text_buffer.
 */
i16 popup_ask(char *txt, char *key, char *dflt, i16 max)
{
    i16 r;
    DosPopup *pop;

    popup_start_ask = 1;
    popup_start_max_len = max;
    popup_start_string = dflt;
    r = 0;
    pop = popup_start_box(txt, key, 0);
    if (pop != 0) {
        r = popup_exec(pop);
        mem_free(pop);
    }
    popup_start_ask = 0;
    return r;
}

/* Asks for a number of up to five digits, initially dflt; leaves it in popup_number and returns
 * popup_ask's result.
 */
i16 popup_ask_number(char *txt, char *key, i16 dflt)
{
    char buf[0x14];
    i16 result;

    msc_itoa(dflt, buf, 10);
    result = popup_ask(txt, key, buf, 5);
    popup_number = msc_atoi(popup_text_buffer);
    return result;
}

/* Reads seven popup colours from the TEXTCOLR sheet: icon n is drawn into a one-pixel port and
 * that pixel is the colour (1 and 2 the background pair, 3 and 4 the selection pair, 5 text,
 * 6 the alternate colour, 7 grey). Then sets the palette from the master palette.
 */
void popup_read_colors(void)
{
    SpriteSeries *colr;
    volatile u8 pix[2];
    DosPort port;

    port.h = port.w = 1;
    port.bits = (void *)pix;
    colr = sprite_series_load("TEXTCOLR", 0);
    if (colr != 0) {
        sprite_draw(1, colr, &port, 0, 0);
        popup_background_color_0 = pix[0];
        sprite_draw(2, colr, &port, 0, 0);
        popup_background_color_1 = pix[0];
        sprite_draw(3, colr, &port, 0, 0);
        popup_select_color_0 = pix[0];
        sprite_draw(4, colr, &port, 0, 0);
        popup_select_color_1 = pix[0];
        sprite_draw(5, colr, &port, 0, 0);
        popup_font_main_color = pix[0];
        sprite_draw(6, colr, &port, 0, 0);
        popup_font_hilite_color = pix[0];
        sprite_draw(7, colr, &port, 0, 0);
        popup_font_grey_color = pix[0];
        mcga_setpal((char *)(msc_vga + 0xfc00));
        mem_free(colr);
    }
}

/* The module's uninitialised state. */
char popup_text_buffer[80];             /* the last entry field's text */
i16 king_frame;                         /* the King's animation frame (0: none) */
i32 king_clock;                         /* the tick the King's next frame is due */
i16 popup_start_max_len;                /* entry field length (0: @LENGTH, or 5) */
i16 popup_number;                       /* popup_ask_number's number */
char *popup_start_string;               /* entry field text (0: @DEFAULT) */
char popup_subst_string[5][0x40];       /* %STRING0..4 */
i32 popup_subst_number[9];              /* %NUMBER0..8 and %HEX0..8 */

