/* menu.c -- menus: the menu bar and its drop-downs, built, drawn and run.
 *
 * A Menu is the bar across the top of the screen. It owns a list of titles (MenuSub), each
 * with its list of items (MenuEntry); all of them, and copies of their texts, live in an
 * arena allocated with the Menu. A title or item text marks its hot key with ~ before the
 * letter ("~Save Map"); "~F~1" marks the function key F1 (menu_get_hotkey). An item with an
 * empty text is a separator. Item flags: 1 disabled (drawn grey, cannot be chosen), 2 hidden;
 * title flag 1 hides the title.
 *
 * The bar and the drop-downs each have a text style (MenuStyle): the extra spacing after each
 * character, the plain colour, the colour of disabled text, the hot-key colour and the font.
 * The colours come from the globals below when the menu is created; the editor sets those
 * from NAMES.TXT's COLORS section (theme.c) before it builds its menu.
 *
 * The editor builds one menu (mapedit.c) and its main loop (input.c) feeds it the mouse and
 * the keyboard: menu_bar_mouse_parse and menu_bar_key_parse open a title's drop-down and run
 * it (menu_bar_run); menu_key_parse takes an item's hot key directly. Each leaves the chosen
 * item's id, or 0, in the Menu's f0.
 */
#include "msc.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */
#include "game.h"
#include "map.h"
#include "pal.h"
#include "popup.h"
#include "buffer.h"
#include "error.h"
#include "font.h"
#include "keys.h"
#include "loader.h"
#include "machine.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "mouse.h"
#include "records.h"
#include "screen.h"
#include "sprite.h"
#include "units.h"
#include "msc.h"
#include "msc.h"

/* The height of a menu row's text in font f: its height, except that 6 counts as 5. */
i16 menu_font_height(DosFont *f)
{
    i16 r;

    r = f->height;
    if (r == 6)
        r--;
    return r;
}
/* Fills a text style: font, spacing after each character, plain, disabled and hot-key colours. */
void menu_set_font(MenuStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi, i16 alt)
{
    st->spacing = spacing;
    st->color = color;
    st->hi = hi;
    st->alt = alt;
    st->font = font;
}
/* buffer_rect_fill, except that colour 7 fills the rectangle from the backdrop tile
 * (menu_tile), when there is one, starting at (a, b) in the tile. c, color2, d and e are
 * unused. */
void menu_fill_rect(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 a, i16 b, i16 c,
                                   char color, char color2, i16 d, i16 e)
{
    if (menu_tile != 0 && color == 7) {
        buffer_tile(port, *menu_tile, x, y, w, h, a, b);
        return;
    }
    buffer_rect_fill(x, y, w, port, h, color);
}
/* The width in pixels of s in style st. Only the first ~ is taken out, so in a text with two
 * (such as "~F~1") the second is measured as a character. */
i16 menu_string_width(MenuStyle *st, char *s)
{
    char buf[0x50];
    char *p;

    msc__fstrcpy(buf, s);
    p = msc_strchr(buf, '~');
    if (p != 0)
        msc__fstrcpy(p, p + 1);
    return font_string_width(st->spacing, st->font, buf);
}
/* Draws s at (x, y) on the work screen in style st and returns the x after it. Normally the
 * text is in the plain colour and the character after each ~ in the hot-key colour. With
 * hilite set (menu_draw_menu passes an item's disabled flag) the whole text is drawn in
 * st->hi, the disabled colour, in one piece: its ~ markers are then drawn as characters. */
i16 menu_string_write(MenuStyle *st, char *s, i16 x, i16 y, i16 hilite)
{
    char one[2];
    char *p;

    if (hilite) {
        font_set_colors(-1, st->hi, st->hi, st->hi);
        x = font_write(&scr_work, x, y, st->font, s, st->spacing) + st->spacing;
    } else {
        one[1] = 0;
        font_set_colors(-1, st->color, st->color, st->color);
        for (p = s; *p != 0; p++) {
            if (*p == '~') {
                font_set_colors(-1, st->alt, st->alt, st->alt);
                p++;
                one[0] = *p;
                x = font_write(&scr_work, x, y, st->font, one, st->spacing) + st->spacing;
                font_set_colors(-1, st->color, st->color, st->color);
            } else {
                one[0] = *p;
                x = font_write(&scr_work, x, y, st->font, one, st->spacing) + st->spacing;
            }
        }
    }
    return x;
}
/* The hot key a menu text marks: the upper-cased character after its last ~, or 0 if it has
 * none. A text with two ~, the first before 'F' and the last before a digit ("~F~1"), marks a
 * function key and returns its key code (0x13b for F1); a '1' just before the last ~ ("~F1~0")
 * makes it F10. Two further tests on the text, a '0' three and five characters past the first
 * ~, switch to the Shift (0x154) and Ctrl (0x15e) function keys; the editor's menus use
 * neither. */
i16 menu_get_hotkey(u8 *s)
{
    i16 r;
    u8 *last;
    u8 *first;

    r = 0;
    last = msc__fstrrchr(s, '~');
    first = msc__fstrchr(s, '~');
    if (first != last && first[1] == 'F' && isdigit(last[1])) {
        r = 0x13b;
        first += 3;
        if (*first == '0') {
            r = 0x154;
            if (first[2] == '0')
                r = 0x15e;
        }
        if (last[-1] == '1')
            r += 9;
        else
            r += last[1] - '1';
    } else if (last != 0) {
        r = islower(last[1]) ? last[1] - 0x20 : last[1];
    }
    return r;
}
/* The colours a new menu takes (menu_create), each a palette index; the _0/_1 pairs are the
 * two colours of a fill, of which only the first is used. Colour 7 means the backdrop tile. */
i16 menu_bar_background_color_0 = 7;      /* the bar */
i16 menu_bar_background_color_1 = 7;
i16 menu_background_color_0 = 7;          /* a drop-down */
i16 menu_background_color_1 = 7;
i16 menu_select_color_0 = 8;              /* the selected item */
i16 menu_select_color_1 = 8;
i16 menu_bar_select_color_0 = 8;          /* the open title */
i16 menu_bar_select_color_1 = 8;
i16 menu_border_color = 0;                /* a drop-down's frame */
i16 menu_font_main_color = 0;             /* item text */
i16 menu_font_grey_color = 8;             /* disabled item text */
i16 menu_font_hilite_color = 0xf;         /* item hot keys */
i16 menu_bar_font_main_color = 0;         /* title text */
i16 menu_bar_font_grey_color = 8;         /* unused: titles are never drawn grey */
i16 menu_bar_font_hilite_color = 0xf;     /* title hot keys */
DosPort *menu_tile = 0;                   /* the backdrop tile, or 0 */

/* Creates an empty menu with an arena of size bytes for its titles, items and texts. Both
 * styles use font; the spacing is fixed (12 pixels between titles, items 3 apart, text 4 in
 * from a drop-down's sides) and the colours are the globals above. Returns 0 when there is no
 * memory. The tests that can never succeed, the switch (0) and the final mem_free (m is
 * always 0 or the result) change nothing.
 *
 * Port: the arena is twice the size asked for. Its records hold pointers, twice as wide on a
 * modern machine, so none needs more than twice its 1994 size. The arena starts after the
 * Menu record, at sizeof (Menu) rather than the 1994 record's fixed size. */
Menu *menu_create(i16 size, DosFont *font)
{
    Menu *result;
    Menu *m;
    Menu *dp;

    result = 0;
    m = mem_get((i32)(2 * (i32)size + sizeof(Menu)));   /* Port: twice size */
    if (m != 0) {
        dp = m;
        heap_declare(0x28, m->arena, (char *)m + sizeof(Menu), 2 * (i32)size);   /* Port: twice */
        m->subs = 0;
        m->n_subs = 0;
        m->title_gap = 0xc;
        m->item_gap = 3;
        m->item_pad = 4;
        m->gap = m->bar_h = 1;
        if (((char *)m + sizeof(Menu)) != ((char *)m + sizeof(Menu))) result = 0;
        if ((u16)size < 0) m = 0;
        if ((u16)size < 0) dp->f0 = 0;
        dp->bar_col = menu_bar_background_color_0;
        dp->bar_col2 = menu_bar_background_color_1;
        menu_set_font(&dp->bar, font, 0, menu_bar_font_main_color, menu_bar_font_grey_color, menu_bar_font_hilite_color);
        dp->title_col = menu_bar_select_color_0;
        dp->title_col2 = menu_bar_select_color_1;
        switch (0) {
        default:
            dp->drop_col = menu_background_color_0;
            dp->drop_col2 = menu_background_color_1;
            dp->sel_col = menu_select_color_0;
            dp->sel_col2 = menu_select_color_1;
            dp->frame_col = menu_border_color;
        }
        menu_set_font(&dp->drop, font, 0, menu_font_main_color, menu_font_grey_color, menu_font_hilite_color);
        result = dp;
    }
    if (m != 0 && m != result)
        mem_free(m);
    if ((u16)size < 0) result = 0;
    return result;
}
/* The title with this id, or 0. */
MenuSub *menu_bar_item(Menu *m, i16 id)
{
    i16 found;
    MenuSub *sub;
    MenuSub *result;

    found = 0;
    result = 0;
    sub = m->subs;
    while (found == 0) {
        if (sub == 0)
            break;
        if (sub->id == id) {
            found = 1;
            result = sub;
        } else
            sub = sub->next;
    }
    return result;
}
/* The item with this id, under any title, or 0. */
MenuEntry *menu_item(Menu *m, i16 id)
{
    i16 found;
    MenuSub *sub;
    MenuEntry *it;
    MenuEntry *result;

    found = 0;
    result = 0;
    sub = m->subs;
    while (found == 0) {
        if (sub == 0)
            break;
        it = sub->items;
        while (found == 0) {
            if (it == 0)
                break;
            if (it->id == id) {
                found = 1;
                result = it;
            } else
                it = it->next;
        }
        sub = sub->next;
    }
    return result;
}
/* Hides (on set) or shows the title with this id; an unknown id is ignored. */
void menu_bar_hide(Menu *m, i16 id, i16 on)
{
    MenuSub *sub;

    sub = menu_bar_item(m, id);
    if (sub != 0) {
        if (on)
            sub->flags |= 1;
        else
            sub->flags &= ~1;
    }
}
/* Disables (on set) or enables the item with this id. The id must exist: the item is not
 * checked for 0. */
void menu_grey(Menu *m, i16 id, i16 on)
{
    MenuEntry *it;

    it = menu_item(m, id);
    if (on)
        it->flags |= 1;
    else
        it->flags &= ~1;
}
/* Enables every item. */
void menu_release_all_grey(Menu *m)
{
    MenuSub *sub;
    MenuEntry *it;

    for (sub = m->subs; sub != 0; sub = sub->next)
        for (it = sub->items; it != 0; it = it->next)
            it->flags &= ~1;
}
/* Hides (on set) or shows the item with this id. As in menu_grey, the id must exist. */
void menu_off(Menu *m, i16 id, i16 on)
{
    MenuEntry *it;

    it = menu_item(m, id);
    if (on)
        it->flags |= 2;
    else
        it->flags &= ~2;
}
/* Shows every item. */
void menu_release_all_off(Menu *m)
{
    MenuSub *sub;
    MenuEntry *it;

    for (sub = m->subs; sub != 0; sub = sub->next)
        for (it = sub->items; it != 0; it = it->next)
            it->flags &= ~2;
}
/* Appends a title to the bar, title_gap pixels after the last one, or against the right edge
 * of the screen when right is set; its width is the text's plus gap on each side. Returns the
 * new title. */
MenuSub *menu_add_bar_item(Menu *m, char *name, i16 id, i16 right)
{
    i16 x;
    MenuSub *s;
    MenuSub *last;

    x = 0;
    last = m->subs;
    for (s = m->subs; s != 0; s = s->next) {
        x = s->x + s->w;
        last = s;
    }
    x += m->title_gap;
    s = heap_get((i32)sizeof(MenuSub), m->arena);   /* Port: the record's own size */
    if (last != 0)
        last->next = s;
    else
        m->subs = s;
    s->prev = last;
    s->next = 0;
    s->items = 0;
    s->n_items = 0;
    msc__fstrcpy(s->name = heap_get((i32)(msc__fstrlen(name) + 1), m->arena), name);
    s->hotkey = menu_get_hotkey(name);
    s->drop_w = 0xa;
    s->x = x;
    s->w = menu_string_width(&m->bar, name) + m->gap * 2;
    if (right)
        s->x = 0x140 - m->title_gap - s->w;
    s->menu = m;
    s->id = id;
    s->flags = 0;
    m->n_subs++;
    return s;
}
/* Appends an item to the title sub_id and widens that drop-down to fit the text with item_pad
 * on each side. An empty name makes a disabled separator. Returns the item, or 0 when there
 * is no such title. */
MenuEntry *menu_add_item(Menu *m, i16 sub_id, char *name, i16 id)
{
    i16 w;
    MenuEntry *it;
    MenuEntry *last;
    MenuSub *sub;

    it = 0;
    sub = menu_bar_item(m, sub_id);
    if (sub != 0) {
        last = 0;
        for (it = sub->items; it != 0; it = it->next)
            last = it;
        it = heap_get((i32)sizeof(MenuEntry), m->arena);   /* Port: the record's own size */
        if (last != 0)
            last->next = it;
        else
            sub->items = it;
        it->prev = last;
        it->next = 0;
        it->flags = 0;
        msc__fstrcpy(it->text = heap_get((i32)(msc__fstrlen(name) + 1), m->arena), name);
        if (*name == 0)
            it->flags |= 1;
        it->id = id;
        it->hotkey = menu_get_hotkey(name);
        w = menu_string_width(&m->drop, name);
        sub->drop_w = max(m->item_pad * 2 + w, sub->drop_w);
        sub->n_items++;
    }
    return it;
}
/* Draws the bar across the top of the work screen, with the title sel (0 for none) on the
 * title colour, and copies it to the video screen when update is set. */
void menu_draw_bar(Menu *m, MenuSub *sel, i16 update)
{
    i16 x;
    i16 h;
    MenuSub *s;

    h = menu_font_height(m->bar.font) + m->bar_h + 1;
    menu_fill_rect(0, 0, 0x140, scr_work, h, 0, 0, 0x140, (char)m->bar_col, (char)m->bar_col2, 0, 0);
    for (s = m->subs; s != 0; s = s->next) {
        if (s->flags & 1)
            continue;
        x = s->x;
        if (s == sel)
            menu_fill_rect(x, 0, s->w + m->gap * 2, scr_work, h, 0, 0, 0x140,
                           (char)m->title_col, (char)m->title_col2, 0, 0);
        menu_string_write(&m->bar, s->name, x + m->gap, m->bar_h, 0);
    }
    if (update)
        mouse_video_update(0, 0, 0, 0, 0x140, h);
}
/* Where the drop-down of sub goes: its box (*px, *py, *pw, *ph) just below the bar at the
 * title's x, moved left and up as needed to end by x 317 and y 199, and the top left of its
 * first item's row (*ix, *iy). The height counts the shown items only. Reports error -80 when
 * the box would start off the screen. */
void menu_compute_size(MenuSub *sub, i16 *px, i16 *py, i16 *pw, i16 *ph, i16 *ix, i16 *iy)
{
    i16 n;
    i16 right;
    i16 bottom;
    MenuEntry *it;
    Menu *m;

    m = sub->menu;
    *px = sub->x;
    *py = menu_font_height(m->bar.font) + m->bar_h + 3;
    n = 0;
    for (it = sub->items; it != 0; it = it->next)
        if ((it->flags & 2) == 0)
            n++;
    *pw = sub->drop_w + 2;
    right = *pw + *px - 1;
    *ph = (menu_font_height(m->drop.font) + m->item_gap) * n + m->item_gap + 2;
    bottom = *ph + *py - 1;
    if (right >= 0x13e)
        *px += 0x13d - right;
    if (bottom >= 0xc6)
        *py += 0xc7 - bottom;
    *ix = *px + 1;
    *iy = m->item_gap + *py + 1;
    if (*px < 0 || *py < 0)
        error_report(-80, 2, 0x28, (i32)*px, (i32)*py);
}
/* Draws the drop-down of sub, with the item sel on the selection colour, and copies it to the
 * video screen. Hidden items are skipped; a separator is a one-pixel line across the box. */
void menu_draw_menu(MenuSub *sub, MenuEntry *sel)
{
    i16 x;
    i16 y;
    i16 w;
    i16 h;
    i16 x1;
    i16 y1;
    i16 w2;
    i16 tx;
    i16 ty;
    i16 yy;
    Menu *m;
    MenuEntry *it;

    m = sub->menu;
    menu_compute_size(sub, &x, &y, &w, &h, &x1, &y1);
    buffer_draw_box(x, y, w + x - 1, scr_work, h + y - 1, (char)m->frame_col);
    x1 = x + 1;
    y1 = y + 1;
    w2 = w - 2;
    menu_fill_rect(x1, y1, w2, scr_work, h - 2, x, y, w, (char)m->drop_col, (char)m->drop_col2, 0, 0);
    it = sub->items;
    tx = x1 + m->item_pad;
    ty = y1 + m->item_gap;
    for (; it != 0; it = it->next) {
        if (it->flags & 2)
            continue;
        if (it == sel)
            menu_fill_rect(x1 + 1, ty - 1, sub->drop_w - 2, scr_work,
                           menu_font_height(m->drop.font) + 2, x, y, w,
                           (char)m->sel_col, (char)m->sel_col2, 0, 0);
        if (*it->text == 0) {
            yy = (menu_font_height(m->drop.font) >> 1) + ty;
            buffer_rect_fill(x1, yy, w2, scr_work, 1, (char)m->drop.color);
        } else
            menu_string_write(&m->drop, it->text, tx, ty, it->flags & 1);
        ty += menu_font_height(m->drop.font) + m->item_gap;
    }
    mouse_video_update(x, y, x, y, w, h);
}
/* Opens the drop-down of sub and runs it until an item is chosen or the menu is cancelled;
 * the chosen item's id, or 0, is left in the menu's f0, and the bar is redrawn closed. A title
 * without items returns at once and leaves f0 as it was. The screen under the drop-down is
 * saved and put back each time it closes.
 *
 * The mouse, with a button down: pointing at another title opens that one, pointing at an
 * enabled item selects it, and pointing anywhere else clears the selection. Releasing the button takes the selected item.
 * With nothing selected, the release cancels, except at the end of the press that opened the
 * menu while the mouse is still on the bar: then the drop-down stays open.
 *
 * The keys (upper-cased first): up and down (also 8 and 2) move to the previous or next item
 * that is enabled, shown and not a separator, wrapping round; left and right open the previous
 * or next shown title, wrapping round; Enter takes the selected item, Esc cancels, and an
 * item's hot key takes it. A drop-down opened while no mouse button is held starts with its
 * first item selected. Pressing and releasing Alt alone also closes the menu and takes the selected item;
 * when the menu was opened with Alt held, the first release of Alt does not count.
 *
 * Port: buffer_preserve gets its mode (-8) and source (-1) as the two ints they are; the
 * original passed them as one long, -8L. */
void menu_bar_run(MenuSub *sub)
{
    i16 row_h1;
    MenuSub *sm;
    i16 sx5;
    i16 ix;
    Menu *m4;
    i16 running;
    i16 y9;
    i16 xj;
    i16 alt1;
    i16 iy;
    i16 save3;
    i16 switch_sub7;
    i16 yy;
    i16 drag8;
    i16 xe9;
    i16 altnav;
    MenuEntry *sel3;
    i16 w;
    i16 h;
    i16 redraw7;
    i16 found;
    i16 key4;
    i16 wrappedj;
    i16 wrapped28;
    i16 alt2j;
    MenuEntry *it;


    altnav = 0;
    sel3 = 0;
    m4 = sub->menu;
    running = 1;
    alt1 = *(u8 *)(msc_bda + 0x0017) & 8;
    do {
        switch_sub7 = 0;
        redraw7 = 1;
        if (sub->items == 0)
            goto done;
        menu_draw_bar(m4, sub, 1);
        menu_compute_size(sub, &xj, &y9, &w, &h, &ix, &iy);
        row_h1 = menu_font_height(m4->drop.font) + m4->item_gap;
        save3 = buffer_preserve(&scr_work, -8, -1, xj, y9, w, h);
        drag8 = mouse_stroke_going;
        if (drag8 == 0)
            sel3 = sub->items;
        do {
            check_music();
            mouse_begin_cycle(0);
            if (mouse_any_stroke) {
                found = 0;
                if (menu_font_height(m4->bar.font) + m4->bar_h + 1 >= mouse_y) {
                    sm = m4->subs;
                    while (sm != 0 && found == 0) {
                        sx5 = sm->x;
                        xe9 = sx5 + sm->w;
                        if (xe9 >= mouse_x && (sm->flags & 1) == 0) {
                            if (sm != sub)
                                found = 1;
                            else
                                sm = 0;
                        } else
                            sm = sm->next;
                    }
                }
                if (found) {
                    sub = sm;
                    switch_sub7 = 1;
                    altnav = 0;
                } else if (mouse_y < y9 || y9 + h - 1 < mouse_y || mouse_x < xj || xj + w - 1 < mouse_x) {
                    redraw7 = 1;
                    sel3 = 0;
                    altnav = 0;
                } else {
                    it = sub->items;
                    found = 0;
                    yy = iy;
                    while (found == 0) {
                        if (it == 0)
                            goto items_done;
                        if ((it->flags & 2) == 0) {
                            if (yy - 1 <= mouse_y && row_h1 + yy - 1 > mouse_y
                                && (it->flags & 1) == 0 && *it->text != 0) {
                                sel3 = it;
                                redraw7 = found = 1;
                                altnav = 0;
                            }
                            yy += row_h1;
                        }
                        it = it->next;
                    }
                items_done:;
                }
            }
            if (keys_any() && running && !switch_sub7) {
                key4 = keys_get();
                if (key4 < 0x100 && islower(key4))
                    key4 -= 0x20;
                altnav = 0;
                switch (key4) {
                case 0x148:
                case '8':
                    wrappedj = wrapped28 = 0;
                    do {
                        if (sel3 != 0)
                            sel3 = sel3->prev;
                        if (sel3 == 0) {
                            sel3 = sub->items;
                            while (sel3->next != 0)
                                sel3 = sel3->next;
                            if (wrappedj)
                                wrapped28 = 1;
                            wrappedj = 1;
                        }
                    } while (((sel3->flags & 3) || *sel3->text == 0) && wrapped28 == 0);
                    redraw7 = 1;
                    break;
                case 0x150:
                case '2':
                    wrappedj = wrapped28 = 0;
                    do {
                        if (sel3 != 0)
                            sel3 = sel3->next;
                        if (sel3 == 0) {
                            sel3 = sub->items;
                            if (wrappedj)
                                wrapped28 = 1;
                            wrappedj = 1;
                        }
                    } while (((sel3->flags & 3) || *sel3->text == 0) && wrapped28 == 0);
                    redraw7 = 1;
                    break;
                case 0x14b:
                    do {
                        sub = sub->prev;
                        if (sub == 0) {
                            sub = m4->subs;
                            while (sub->next != 0)
                                sub = sub->next;
                        }
                    } while (sub->flags & 1);
                    switch_sub7 = 1;
                    break;
                case 0x14d:
                    do {
                        sub = sub->next;
                        if (sub == 0)
                            sub = m4->subs;
                    } while (sub->flags & 1);
                    switch_sub7 = 1;
                    break;
                case 0xd:
                    running = 0;
                    break;
                case 0x1b:
                    running = 0;
                    sel3 = 0;
                    break;
                default:
                    found = 0;
                    it = sub->items;
                    while (found == 0) {
                        if (it == 0)
                            goto hot_done;
                        if (it->hotkey == key4 && (it->flags & 3) == 0)
                            found = 1;
                        else
                            it = it->next;
                    }
                hot_done:
                    if (found) {
                        sel3 = it;
                        running = 0;
                        redraw7 = 1;
                    }
                    break;
                }
            }
            alt2j = *(u8 *)(msc_bda + 0x0017) & 8;
            if (altnav && alt2j == 0) {
                if (alt1 == 0)
                    running = 0;
                alt1 = 0;
            }
            if (alt2j && !redraw7 && !switch_sub7)
                altnav = 1;
            else
                altnav = 0;
            if (redraw7 && !switch_sub7) {
                menu_draw_menu(sub, sel3);
                redraw7 = 0;
            }
            if (mouse_stop_stroke) {
                if (sel3 != 0)
                    ;
                else
                    goto click_check;
            click_stop:
                running = 0;
            click_keep:
                drag8 = 0;
            }
            mouse_end_cycle(0, running);
        } while (running && !switch_sub7);
        buffer_restore(&scr_work, save3, -1, xj, y9, w, h);
        mouse_video_update(xj, y9, xj, y9, w, h);
    } while (running);
    if (sel3 != 0) {
        m4->f0 = sel3->id;
        goto have_f0;
click_check:
        if (drag8 == 0)
            goto click_stop;
        if (menu_font_height(m4->bar.font) + m4->bar_h + 1 < mouse_y)
            goto click_stop;
        goto click_keep;
    } else
        m4->f0 = 0;
have_f0:
done:
    mouse_init_cycle();
    menu_draw_bar(m4, 0, 1);
}
/* Clears f0 and checks whether a mouse press has just started on a title: 1 if so, and with
 * mode set the title's drop-down is run (menu_bar_run). The press hits the first shown title
 * whose right edge is at or right of the mouse, so a press in the gap before a title opens
 * it. */
i16 menu_bar_mouse_parse(Menu *m, i16 mode)
{
    i16 h;
    i16 found;
    i16 r;
    MenuSub *s;

    r = 0;
    m->f0 = 0;
    if (mouse_start_stroke) {
        h = menu_font_height(m->bar.font) + m->bar_h + 1;
        if (h >= mouse_y) {
            found = 0;
            s = m->subs;
            while (s != 0 && found == 0) {
                if (s->x + s->w >= mouse_x && (s->flags & 1) == 0)
                    found = 1;
                else
                    s = s->next;
            }
            if (found) {
                r = 1;
                if (mode)
                    menu_bar_run(s);
            }
        }
    }
    return r;
}
/* Clears f0; if key is Alt+letter and a shown title has that hot key, runs its drop-down and
 * returns 1, else 0. keys_fix_alt also upper-cases a plain lower-case letter, which would then
 * count as Alt+letter; the main loop upper-cases its keys before they get here. */
i16 menu_bar_key_parse(Menu *m, i16 key)
{
    i16 k;
    i16 found;
    i16 r;
    MenuSub *s;

    s = m->subs;
    r = 0;
    m->f0 = 0;
    found = 0;
    k = keys_fix_alt(key);
    if (k != key) {
        while (s != 0 && found == 0) {
            if (s->hotkey == k && (s->flags & 1) == 0)
                found = 1;
            else
                s = s->next;
        }
    }
    if (found) {
        menu_bar_run(s);
        r = 1;
    }
    mouse_init_cycle();
    return r;
}
/* Chooses the first enabled, shown item, under a shown title, whose hot key is key: returns
 * its id and leaves it in f0, or 0. This is how the function-key items work without opening
 * the menu; a letter matches too. */
i16 menu_key_parse(Menu *m, i16 key)
{
    i16 found;
    i16 r;
    MenuSub *s;
    MenuEntry *it;

    it = 0;
    s = m->subs;
    r = 0;
    m->f0 = 0;
    found = 0;
    for (; s != 0 && found == 0; s = s->next) {
        if (s->flags & 1)
            continue;
        it = s->items;
        while (it != 0 && found == 0) {
            if (it->hotkey == key && (it->flags & 3) == 0)
                found = 1;
            else
                it = it->next;
        }
    }
    if (found) {
        r = it->id;
        m->f0 = r;
    }
    return r;
}
/* Reads the menu colours from MENUCOLR.SS, one from the pixel of each of its first seven
 * sprites: the backgrounds, their second colours, the selection, its second colour, the text,
 * the hot keys and the disabled text, each for both the bar and the drop-downs. The seventh
 * goes to menu_bar_font_main_color where menu_bar_font_grey_color is meant, as in the original,
 * so the title text takes the disabled colour and the bar's disabled colour keeps its default.
 * The editor does not call this; it takes its colours from NAMES.TXT. */
void menu_read_colors(void)
{
    u8 pix;
    void *colr;
    DosPort port;

    port.h = port.w = 1;
    port.bits = &pix;
    colr = sprite_series_load("MENUCOLR.SS", 0);
    if (colr != 0) {
        sprite_draw(1, colr, &port, 0, 0);
        menu_background_color_0 = menu_bar_background_color_0 = pix;
        sprite_draw(2, colr, &port, 0, 0);
        menu_background_color_1 = menu_bar_background_color_1 = pix;
        sprite_draw(3, colr, &port, 0, 0);
        menu_select_color_0 = menu_bar_select_color_0 = pix;
        sprite_draw(4, colr, &port, 0, 0);
        menu_select_color_1 = menu_bar_select_color_1 = pix;
        sprite_draw(5, colr, &port, 0, 0);
        menu_font_main_color = menu_bar_font_main_color = pix;
        sprite_draw(6, colr, &port, 0, 0);
        menu_font_hilite_color = menu_bar_font_hilite_color = pix;
        sprite_draw(7, colr, &port, 0, 0);
        menu_font_grey_color = menu_bar_font_main_color = pix;
        mem_free(colr);
    }
}

