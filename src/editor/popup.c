/* From mapedit-decomp src/editor/popup (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "sprite.h"
/* MODULE popup: MAPEDIT.EXE segment 033d (popup.obj), file 0x49de-0x8378, 64 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn popup_set_dude_name     0x049de
 * fn popup_dude_tribe        0x04a20
 * fn popup_dude_ms           0x04aa0
 * fn popup_dude_myr          0x04aca
 * fn popup_load_dude         0x04af4
 * fn popup_fill              0x04d1a
 * fn popup_say_string        0x04dae
 * fn popup_say               0x04dca
 * fn popup_country           0x04de2
 * fn popup_num               0x04e0a
 * fn popup_set_font          0x04e24
 * fn style_text_width        0x04e64
 * fn style_color             0x04ed4
 * fn style_draw_text         0x04f16
 * fn hotkey_of               0x05020
 * fn popup_create            0x050ae
 * fn Popup_find_item         0x05278
 * fn popup_grey              0x052d8
 * fn popup_hilite            0x05308
 * fn popup_release_all_grey  0x05338
 * fn popup_read_check        0x05368
 * fn popup_write_check       0x05398
 * fn popup_set_active_item   0x053c0
 * fn popup_add_item          0x053de
 * fn popup_add_check         0x055c6
 * fn popup_set_width         0x05600
 * fn popup_add_string        0x05610
 * fn popup_add_entry         0x05722
 * fn font_height             0x058f4
 * fn popup_add_sprite        0x0591a
 * fn popup_add_cursor        0x05b08
 * fn popup_draw_line         0x05b4a
 * fn popup_wrap_lines        0x05b76
 * fn popup_layout            0x05ea4
 * fn popup_show              0x0641a
 * fn popup_help_draw         0x06456
 * fn popup_big_sprite        0x064c6
 * fn popup_draw_items        0x0655a
 * fn popup_draw_edits        0x067f2
 * fn popup_draw_sprites      0x069fc
 * fn popup_box_draw          0x06c56
 * fn popup_draw              0x06e6c
 * fn popup_set_active_sprite 0x06f3c
 * fn popup_exec              0x06f5e
 * fn popup_init              0x07a62
 * fn popup_parse_string      0x07a7a
 * fn popup_start_box         0x07c82
 * fn popup_box               0x080a8
 * fn pop_clear               0x080da
 * fn pop_set                 0x080e2
 * fn pop_get                 0x0810c
 * fn pop                     0x08122
 * fn pop2                    0x08130
 * fn popi                    0x0813e
 * fn pop2i                   0x08156
 * fn popk                    0x08168
 * fn popm                    0x08180
 * fn pop2m                   0x08198
 * fn popf                    0x081aa
 * fn pop2f                   0x081c2
 * fn popup_off               0x081d4
 * fn popup_ask               0x081da
 * fn popup_ask_number        0x08226
 * fn popup_read_colors       0x08268
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl23_6be50). The two functions the draft dropped are VICEROY's code under the
 * editor's names: popup_num (set_text_arg_l: popup_subst_number[slot] = v) and
 * popup_parse_string (expand_text_template, the %STRING/%NUMBER/%HEX/%COUNTRY/%YEAR/%%
 * expander). Their four symbols restored the mod-17 counts of the eight functions that
 * DIFFERed after them (popup_layout .. popup_start_box), so the VICEROY pads hold here too.
 * Globals and callees carry their CodeView names (the DS: comments are the editor's
 * addresses): popup_subst_string/number, popup_indian/foreign/messenger, popup_help(_me),
 * popup_checkbox, popup_start_string/max_len, popup_text_buffer, the popup_*_color words,
 * king_frame/clock/active, scr_work/scr_orig, mouse_* and keys_*, buffer_draw_box/hline/vline,
 * buffer_preserve/restore, error_report, load_main_sprites, menu_font, game and unit.
 * popup_help_draw's g_popup_cancel_button is popup_help; the "~ hotkey colour" is hilite_color.
 *
 * THE 1994 POPUP MODULE: the DOS counterpart of the Windows Popup class (1050:0000-6600) and
 * its text-dialog helpers (1050:5c00-658f), rebuilt as a C struct (DosPopup) drawn straight
 * onto the 320x200 screen port. The 20
 * `jmp far` stubs at 0x6f7ea are the `push cs; call near` targets for same-file far calls.
 *
 * WHAT THE DOS BUILD DOES DIFFERENTLY (findings):
 *   - Portraits ("dudes"): KING / IND<tribe><mood> (mood = piss_factor(pissed(tribe,
 *     player))) / MSS<n> / MYR<n> sprite sheets are loaded per popup (popup_load_dude), with a
 *     second animated sheet for the King that popup_exec steps every 10 ticks.
 *   - popup_start_box parses the .TXT section itself: @OPTIONS/@PROMPT, @TEXT, @SMALLFONT,
 *     @Y, @X, @WIDTH, @LENGTH, @CHECKBOX, @DEFAULT (=text for edit prompts, a number for the
 *     preselected item when the caller passed 0), any other @-line ends the dialog.
 *   - Hotkeys: the character after '~' (upper-cased); "~F~n" is function key F<n> (0x10a+'n').
 *     '{' '}' switch the alternate colour, '|' right-aligns the rest of an item.
 *   - Check boxes: pop_set/pop_get take a 1-based item number (Windows 0-based);
 *     popup_exec loads item values from popup_checkbox on entry and stores them back on exit.
 *   - popup_exec: Escape returns -1; F1 sets popup_help_me when the popup has the help
 *     button; in autoplay (automate) a popup closes itself after 120 ticks; clicking outside
 *     the items keeps the popup open; a sprite list (unit picker) clears the unit's orders
 *     instead of closing when the picked unit has orders.
 *   - popup_read_colors reads the seven popup colours from the TEXTCOLR sheet by drawing each
 *     icon into a one-pixel port.
 *
 * PLUMBING: medium model C; near _fastcall helpers inside the module (ret n); heap_get
 * (181f:002c) allocates out of the popup's own arena at +0x84; mem_free is the generic far
 * free; popup_exec is pascal and defined here (popup_exec).
 *
 *   - NEW: the declared TYPE of a _fastcall register parameter changes the order the register
 *     arguments are evaluated in: buffer_draw_box(unsigned x0, ...) and sprite_draw(unsigned icon,
 *     ...) load ax first, as shipped (popup_box_draw went from 161 differing bytes to 0).
 *   - NEW: `while (*p) { if (!c) { p++; continue; } break; }` gives the shipped bottom-tested
 *     loop; `while (*p) { if (c) break; p++; }` and `for (; *p; )` stay top-tested.
 *   - NEW: MSC's toupper spelling `islower(c) ? _toupper(c) : c` costs a ?: temp slot
 *     (hotkey_of's frame is 2 bytes larger than its locals).
 *   - NEW: a far pointer copied from a `long` (popup_create's m) is not copy-propagated, so
 *     the copy keeps its own slot as shipped.
 *   - NEW: `volatile` on a local pixel buffer reached through a port struct forces the
 *     shipped reload after each sprite_draw (popup_read_colors).
 *   - Changing struct/typedef declarations before a function shifts the period-17 counts:
 *     unifying DosDude into DosSprite removed the need for three pad blocks.
 *   - popup_layout needs 10 pads (pad_popup_layout_*).
 *
 *  All three sites came from the
 *   control flow, not from how the statements are spelled:
 *   - `if (m == 0) goto skip; { ... } skip:` instead of `if (m != 0) { ... }`. Inside an if
 *     block whose test is the just-assigned allocation result (m, or dp, or `(dp = ...) != 0`),
 *     MSC stores every constant at once; after a goto (or an early return, or a test of an
 *     unrelated value such as size) it DELAYS the run of zero stores: the 17 int zeros sink
 *     past the five popup_set_font colour pushes and `f32 = list_rows = 4` goes through `mov ax,4`
 *     -- both shipped shapes, from the one change. Snippets: the same 25 stores + call in a
 *     function with dp a parameter sink; with dp a local assigned from the allocation and
 *     tested in an enclosing if they do not.
 *   - heap_declare's arguments: `e = (char far *)m + 0x96; a = dp->arena;
 *     heap_declare(0x29, a, e, size);` (the two spare frame locals; e first).
 *   - With the stores delayed, `dp->flags = popup_flags` was delayed too (its value reused
 *     for the f46/f48 tests); shipped stores it at once and reloads the byte: a volatile
 *     STORE, `(*(volatile int far *)&dp->flags) = popup_flags;`. A volatile read at f46 or
 *     an element read do not.
 *   - The label is one symbol: 16 pads after the function (pad_pc_after_*) keep every later
 *     function's count.
 *   Before: the e/a temps alone cost 3 bytes (461); `list_rows = 4; f32 = list_rows;`,
 *   `(int)4`, a chain inside the spacing argument, pads 0..16, and the struct-by-value colours
 *   idea (untried, no longer needed) were the earlier attempts.
 *
 *  The volatiles are gone; the three sites of the
 *   non-volatile state each had their own lever:
 *   - `tots = eh_ + ih9 + th` started from ih9: the chain order follows the locals' declaration
 *     positions mod 17, and only ONE residue of eh_'s position (relative to ih9) starts from
 *     eh_. Reordering declarations moves slots (bucket rule), so the shift is done with
 *     BLOCK-SCOPE `extern int` lines inside the declaration list: each is a symbol with no
 *     frame slot.
 *   - `p->r_h = max(nhv, dh)` loaded nhv first: 11..15 block externs before dh flip it; 11
 *     before dh plus 8 before eh_ (19 = 2 mod 17 for eh_) fixes both sites.
 *   - case 1's `p->r_x = min(p->x, spr5->idx)` loaded the member twice (the 2-byte length
 *     difference): read p->x into the unused u3s BEFORE the idx store,
 *     `u3s = p->x; spr5->idx = 160 - (dw1 >> 1); p->r_x = min(u3s, spr5->idx);` -- the store
 *     to u3s is dropped and the member is loaded once into cx as shipped. Reading it after the
 *     idx store costs 4; extern shifts of every local (a14+k, spr5+k) never moved this site.
 *   - The 19 block externs count for later functions: 15 pads after the function
 *     (pad_pl_after_*); 16 also keeps all 64.
 *   Earlier attempts: all 211 volatile subsets up to size
 *   two, pads 0..16, min's argument order, ternary/if forms, element reads, spr5 at each of
 *   21 positions, the min/max statement order, idx in u2s, per-function pragmas.
 *
 *  Each icon argument is an assignment
 *   to its OWN otherwise unused local: `sprite_draw(vx = 1, ...)` with `unsigned vx`, and
 *   `sprite_draw(wx = king_frame, ...)` with `int wx`, plus one more unused int (uy) to keep the
 *   frame. One shared vx left 1 byte; `int vx` folds `vx = 1` back to the constant (dx before
 *   ax). The stores are dead and dropped; the assignment only makes ax the computed operand. `ox = b + p->x + p->f48` became
 *   `ub = b + p->x; oyn = b + p->y; ox = ub + p->f48;` -- a single-use intermediate local
 *   (ub, unused before) gives the shipped interleaving; `ox += p->f48` after oyn, comma forms,
 *   chained `ox = oyn = b`, volatile/element views of x or f48, and pads 0..16 did not.
 *
 *  What placed the blocks:
 *   - MSC lays out the arm a switch dispatch falls into and parks the other arms at the next
 *     unconditional jump (here inside `if (fullv) popup_draw(p); else ...`). In the item
 *     switch the arm that stays is case 0x1b, so it comes FIRST; the rest keep their order.
 *   - The 0x150/0x148 arms end in `goto set_redraw` to the ' ' arm's `redraw = 1; break;`:
 *     shipped keeps that later copy (a plain `redraw = 1; break;` keeps the first).
 *   - An inverted if puts the reload where shipped has it: `if (p->first_sprite == 0) spr4 =
 *     p->sprites; else {...}` reloads p at the else and only es at the join, and `if (p->flags
 *     & 4) toggle; else runningx = 0;` reuses dx:ax for `it`. The branch generated second
 *     starts from what the first left in the registers.
 *   - The click handler's `if (!sel) runningx = 0; else if (flags & 4) {...} else runningx =
 *     0;` -- the duplicate tail -- is what moves the toggle out of line.
 *   - The hotkey loop leaves with `goto hot_end` (a `break` is hoisted into the loop test).
 *   - The King frame test reads the sheet header's word at +4, not icon[0]'s; the F1 branch
 *     does not repeat `runningx = 0`.
 */


#include "msc.h"
#include "msc.h"
#include "msc.h"
#include "records.h"

typedef struct { u8 height; } DosFont;

/* The popup module's initialised data, defined here (the 27 land at DS:0x053c-0x0572 in this
 * order, ahead of the "KING" literal: MSC emits a global after the literals of the function
 * that follows its definition, and popup_set_dude_name has none). Each was first declared by
 * an extern further down; those lines are now unused pads of the same count, and 41 more pads
 * here bring the symbols added at this point to 0 mod 17, so every function keeps its count. */
i16 popup_background_color_0 = 7;       /* DS:0x053c */
i16 popup_background_color_1 = 7;       /* DS:0x053e */
i16 popup_select_color_0 = 8;           /* DS:0x0540 */
i16 popup_select_color_1 = 8;           /* DS:0x0542 */
i16 popup_border_color = 0;             /* DS:0x0544 */
i16 popup_border_color_up = 0;          /* DS:0x0546 */
i16 popup_border_color_down = 0;        /* DS:0x0548 */
i16 popup_font_main_color = 0;          /* DS:0x054a */
i16 popup_font_grey_color = 8;          /* DS:0x054c */
i16 popup_font_hilite_color = 0xf;      /* DS:0x054e */
i16 popup_enhance_color = 7;            /* DS:0x0550 */
i16 popup_shadow_color = 8;             /* DS:0x0552 */
i16 popup_checkbox = 0;                 /* DS:0x0554 */
i16 popup_flags = 0;                    /* DS:0x0556 */
i16 popup_x = -1;                       /* DS:0x0558 */
i16 popup_y = -1;                       /* DS:0x055a */
i16 popup_indian = -1;                  /* DS:0x055c */
i16 popup_messenger = -1;               /* DS:0x055e */
i16 popup_foreign = -1;                 /* DS:0x0560 */
i16 popup_hiliting = 0;                 /* DS:0x0562 */
i16 popup_pal = 1;                      /* DS:0x0564 */
i16 popup_help = 0;                     /* DS:0x0566 */
i16 popup_help_me = 0;                  /* DS:0x0568 */
i16 popup_no_draw = 0;                  /* DS:0x056a */
DosPort *popup_tile = 0;                /* DS:0x056c */
i16 king_active = 0;                    /* DS:0x056e */
i16 sprite_tight = 0;                   /* DS:0x0570 */
/* A text style: the extra spacing after each character, the plain colour, the colour of a
 * highlighted line, the colour of a {}-marked span, two more colours font_set_colors takes, the font. */
typedef struct { i16 spacing; i16 color; i16 hi_color; i16 alt_color; i16 c8; i16 ca; DosFont *font; } TextStyle;
/* A popup item: flags (1 grey, 2 hilite, 4 checked), the ~ hotkey, its key and value, its
 * own copy of the text out of the popup's arena, and the list links. 0x18 bytes. */
typedef struct DosItem {
    i16 flags; i16 hotkey; i16 key; i16 value; char *text; i16 fc; i16 fe;
    struct DosItem *next; struct DosItem *prev;
} DosItem;
/* A text line: flags (1 centred, 2 ...), the text, the next line. 10 bytes. */
typedef struct DosLine { i16 flags; char *text; struct DosLine *next; } DosLine;
/* An edit field: flags (0x80 has text), the label's width, the field's width, the length,
 * the label (with a space added), the buffer, the next field. 0x14 bytes. */
typedef struct DosEdit {
    i16 flags; i16 label_w; i16 field_w; i16 len; char *label; char *buf;
    struct DosEdit *next;
} DosEdit;
/* A sprite item: its x, the x after it, the icon index, the key, the label, the icon sheet,
 * the next sprite. 0x14 bytes. */

typedef struct DosSprite {
    i16 x; i16 x2; i16 idx; i16 key; char *label; SpriteSeries *sheet;
    struct DosSprite *next;
} DosSprite;
typedef struct { i16 f0; i16 f2; i16 b; i16 f6; i16 f8; i16 fa; void *a; } DosAttach;
typedef struct {
    i16 f0; i16 n_items; i16 n_lines; i16 n_sprites; i16 n_edits; i16 flags; i16 pos_x; i16 pos_y;
    i16 x; i16 y; i16 w; i16 h; i16 r_x; i16 r_y; i16 r_w; i16 r_h;
    i16 item_w; i16 list_rows; i16 f24; i16 f26; i16 width; i16 f2a; i16 f2c; i16 spr_h;
    i16 spr_lw; i16 f32; i16 edit_w; i16 f36; i16 f38; i16 f3a; i16 col0; i16 col1;
    i16 col2; i16 col3; i16 col4; i16 f46; i16 f48; i16 f4a;
    DosItem *sel_item; DosSprite *first_sprite; DosItem *items; DosLine *lines;
    DosSprite *sprites; DosEdit *edits; DosAttach *attach;
    DosSprite *dude; char *dude_name; void *f70;
    TextStyle style; Arena arena[1];   /* PORT: the port's Arena (0x12 bytes in 1994) */
} DosPopup;

extern void * heap_get(i32 n, void *arena);
extern GameState game;
extern i16 king_frame;
extern i32 king_clock;
extern i32 timer_read(void);
extern i16 pissed(i16 tribe, i16 nation);
extern i16 piss_factor(i16 tension);
/* popup_set_dude_name -- 033d:000e, 66 bytes */
void popup_set_dude_name(char *name, DosPopup *p)
{
    p->dude_name = heap_get((i32)(msc_strlen(name) + 1), p->arena);
    msc__fstrcpy(p->dude_name, name);
}
/* popup_dude_tribe -- 033d:0050, 128 bytes */
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
/* popup_dude_ms -- 033d:00d0, 42 bytes */
void popup_dude_ms(DosPopup *p)
{
    char name[20];

    msc_strcpy(name, "MSS0");
    name[3] += popup_messenger;
    popup_set_dude_name(name, p);
}
/* popup_dude_myr -- 033d:00fa, 42 bytes */
void popup_dude_myr(DosPopup *p)
{
    char name[20];

    msc_strcpy(name, "MYR0");
    name[3] += popup_foreign;
    popup_set_dude_name(name, p);
}
/* popup_load_dude -- 033d:0124, 550 bytes */
extern i16 cycling_active;                     /* DS:0x3c64 */
extern char *put_palette;            /* DS:0x3ae0 */
extern void cycle_save(i16 on);
extern void reset_sprite_memory(void);
extern void * sprite_load(char *name, i16 a);
extern void mem_free(void *p);
extern void mcga_setpal(char *pal);

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
    p->dude = heap_get((i32)sizeof(DosSprite), p->arena);   /* PORT: 20 bytes in 1994 */
    dd = p->dude;
    dd->next = 0;
    if (king) {
        second = heap_get((i32)sizeof(DosSprite), p->arena);   /* PORT: 20 bytes in 1994 */
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
/* popup_fill -- 033d:034a, 148 bytes */
i16 orig_flag = 0;                       /* DS:0x1f8a (defined here: lands after popup_load_dude's literals) */
extern DosPort scr_orig;               /* DS:0x3afc */
extern void buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h);
extern void buffer_tile(DosPort dst, DosPort src, i16 dx, i16 dy, i16 w, i16 h, i16 sx, i16 sy);
extern void buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color);

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
/* popup_say_string -- 033d:03de, 28 bytes */
extern char popup_subst_string[][0x40];     /* DS:0x634e */
extern i32 popup_subst_number[];           /* DS:0x698a */
extern char *strings(i16 n);
extern void say_country(i16 nation, i16 flag, char *s);

void popup_say_string(i16 slot, char *s)
{
    msc__fstrcpy(popup_subst_string[slot], s);
}
/* popup_say -- 033d:03fa, 24 bytes */
void popup_say(i16 slot, i16 word)
{
    popup_say_string(slot, strings(word));
}
/* popup_country -- 033d:0412, 40 bytes */
void popup_country(i16 slot, i16 flag, i16 nation)
{
    char buf[0x50];

    buf[0] = 0;
    say_country(nation, flag, buf);
    popup_say_string(slot, buf);
}
/* popup_num -- 033d:043a, 26 bytes */
void popup_num(i16 slot, i32 v)
{
    popup_subst_number[slot] = v;
}
/* popup_set_font -- 033d:0454, 64 bytes */
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
/* style_text_width -- 033d:0494, 112 bytes */
extern i16 font_string_width(i16 flags, DosFont *font, char *s);

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
/* style_color -- 033d:0504, 66 bytes */
extern void font_set_colors(i16 a, i16 b, i16 c, i16 d);

void style_color(i16 hi, i16 alt, TextStyle *st)
{
    if (hi)
        font_set_colors(-1, st->hi_color, st->c8, st->ca);
    else if (alt)
        font_set_colors(-1, st->alt_color, st->c8, st->ca);
    else
        font_set_colors(-1, st->color, st->c8, st->ca);
}
/* style_draw_text -- 033d:0546, 266 bytes */
extern DosPort scr_work;               /* DS:0x3af4 */
extern i16 font_write(DosPort *port, i16 x, i16 y, DosFont *font, char *s, i16 flags);

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
/* hotkey_of -- 033d:0650, 142 bytes */
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
/* popup_create -- 033d:06de, 458 bytes */
extern i16 mouse_driver;                     /* DS:0x49c0 */
extern void * mem_get(i32 n);
extern void heap_declare(i16 kind, void *arena, void *base, i32 size);

DosPopup *popup_create(i16 size, DosFont *font)
{
    DosPopup *result;
    char *m;   /* PORT: the block, not a long (a pointer is not 32 bits) */
    DosPopup *dp;
    void *a;
    void *e;

    result = 0;
    king_active = 0;
    sprite_tight = 0;
    /* PORT: the arena twice the 1994 size -- its records hold pointers, twice as wide now
       (and /Zp2 adds no padding), so none is more than twice what it was */
    m = (char *)mem_get((i32)(2 * (i32)(u16)size + sizeof(DosPopup)));   /* PORT: 0x96 bytes in 1994 */
    if (m == 0)
        goto skip;
    {
        dp = (DosPopup *)m;
        e = m + sizeof(DosPopup);   /* PORT */
        a = dp->arena;
        heap_declare(0x29, a, e, 2 * (i32)(u16)size);   /* PORT */
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
    if (m != 0 && (DosPopup *)m != result)
        mem_free((void *)m);
    return result;
}
/* Popup_find_item -- 033d:08a8, 96 bytes */
/* 16 pads: popup_create's `skip:` label is one symbol (17 - 1). */
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
/* popup_grey -- 033d:0908, 48 bytes */
void popup_grey(DosPopup *p, i16 key, i16 on)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (on)
        it->flags |= 1;
    else
        it->flags &= ~1;
}
/* popup_hilite -- 033d:0938, 48 bytes */
void popup_hilite(DosPopup *p, i16 key, i16 on)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (on)
        it->flags |= 2;
    else
        it->flags &= ~2;
}
/* popup_release_all_grey -- 033d:0968, 48 bytes */
void popup_release_all_grey(DosPopup *p)
{
    DosItem *it;

    it = p->items;
    while (it != 0) {
        it->flags &= ~1;
        it = it->next;
    }
}
/* popup_read_check -- 033d:0998, 48 bytes */
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
/* popup_write_check -- 033d:09c8, 40 bytes */
void popup_write_check(DosPopup *p, i16 key, i16 v)
{
    DosItem *it;

    it = Popup_find_item(key, p);
    if (it != 0)
        it->value = v;
}
/* popup_set_active_item -- 033d:09f0, 30 bytes */
void popup_set_active_item(DosPopup *p, i16 key)
{
    p->sel_item = Popup_find_item(key, p);
}
/* popup_add_item -- 033d:0a0e, 488 bytes */
#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

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
    item = heap_get((i32)sizeof(DosItem), p->arena);   /* PORT: 0x18 bytes in 1994 */
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
/* popup_add_check -- 033d:0bf6, 58 bytes */
DosItem *popup_add_check(DosPopup *p, char *s, i16 key, i16 on)
{
    DosItem *it;

    p->flags |= 5;
    it = popup_add_item(p, s, key);
    if (it != 0)
        it->value = on;
    return it;
}
/* popup_set_width -- 033d:0c30, 16 bytes */
void popup_set_width(DosPopup *p, i16 w)
{
    p->width = w;
}
/* popup_add_string -- 033d:0c40, 274 bytes */
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
    item = heap_get((i32)sizeof(DosLine), p->arena);   /* PORT: 10 bytes in 1994 */
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
/* popup_add_entry -- 033d:0d52, 466 bytes */
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
    ed = heap_get((i32)sizeof(DosEdit), p->arena);   /* PORT: 0x14 bytes in 1994 */
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
/* font_height -- 033d:0f24, 38 bytes */
i16 font_height(DosFont *c)
{
    i16 r;

    r = c->height;
    if (r == 6 && orig_flag == 0)
        r = 5;
    return r;
}
/* popup_add_sprite -- 033d:0f4a, 494 bytes */
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
    spr = heap_get((i32)sizeof(DosSprite), p->arena);   /* PORT: 0x14 bytes in 1994 */
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
/* popup_add_cursor -- 033d:1138, 66 bytes */
DosAttach *popup_add_cursor(DosPopup *p, void *a, i16 b)
{
    p->attach = heap_get((i32)sizeof(DosAttach) > 0x14 ? (i32)sizeof(DosAttach) : 0x14L, p->arena);   /* PORT: 0x14 bytes in 1994 */
    p->attach->a = a;
    p->attach->b = b;
    return p->attach;
}
/* popup_draw_line -- 033d:117a, 44 bytes */
i16 popup_draw_line(i16 x, i16 y, DosPopup *p, char *s)
{
    i16 r;

    return style_draw_text(x + (p->f48 + p->f2a), y, 0, &p->style, s);
}
/* popup_wrap_lines -- 033d:11a6, 814 bytes */
/* Lays the text lines out in the popup's width, word by word, drawing them when `draw` is
 * set; returns the height they take. Lines flagged 1 (centred) or 2 are not wrapped. */
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
/* popup_layout -- 033d:14d4, 1398 bytes */
extern DosFont *menu_font;                 /* DS:0x0080 */
extern void error_report(i16 code, i16 a, i16 b, i32 c, i32 d);

/* Sizes and places the popup: its box from the sprites, the wrapped text, the items and the
 * edit fields; centred on the 320x200 screen unless a position was given, then moved to make
 * room for the dude's portrait. Returns 1 when the popup is empty. */
/* 16 pads here (14 before the data was defined at the top) and 16 before popup_exec (15). */
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
    /* Block-scope externs: symbols with no frame slot. 11 here put dh (and everything
     * after) where `max(nhv, dh)` loads dh first; 8 more put eh_ where `eh_ + ih9 + th`
     * starts from eh_. See the module header. */
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
/* popup_show -- 033d:1a4a, 60 bytes */
/* 15 pads: popup_layout's 19 block-scope externs are 19 symbols (19 + 15 = 34 = 2 x 17). */

extern void mouse_video_update(i16 sx, i16 sy, i16 dx, i16 dy, i16 w, i16 h);

void popup_show(DosPopup *p)
{
    if (popup_no_draw == 0 || (p->flags & 0x20) == 0)
        mouse_video_update(p->r_x, p->r_y, p->r_x, p->r_y, p->r_w, p->r_h);
}
/* popup_help_draw -- 033d:1a86, 112 bytes */
extern i16 spot[];                          /* DS:0x530a (hspot_1.c's HotSpot spot[121]): the help word is
                                             * read from DS:0x53b4, spot[10].v2 -- no public lies there */
extern u8 hilite_color;          /* DS:0x0093 */
extern void say_string(char *buf, i16 word);
extern void write_right(char *s, i16 x, i16 y, i16 color);

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
        say_string(buf, spot[0x55]);
        write_right(buf, x, yv, hilite_color);
    }
}
/* popup_big_sprite -- 033d:1af6, 148 bytes */

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
/* popup_draw_items -- 033d:1b8a, 664 bytes */
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
/* popup_draw_edits -- 033d:1e22, 522 bytes */

extern void buffer_draw_box(u16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color);

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
/* popup_draw_sprites -- 033d:202c, 602 bytes */
extern void piece_draw(i16 u, i16 flags, i16 x, i16 y, i16 span, i16 scale);

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
/* popup_box_draw -- 033d:2286, 534 bytes */
extern void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color);
extern void buffer_hline(i16 x0, i16 x1, i16 y, DosPort port, char color);

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
/* popup_draw -- 033d:249c, 208 bytes */
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
/* popup_set_active_sprite -- 033d:256c, 34 bytes */
void popup_set_active_sprite(DosPopup *p, DosSprite *spr)
{
    p->first_sprite = spr;
    popup_draw_sprites(0, 1, 1, p);
}
/* popup_exec -- 033d:258e, 2820 bytes */
extern Unit unit[];                      /* DS:0x521e */
extern char popup_text_buffer[];                /* DS:0x4b64 */
extern i16 mouse_x, mouse_y;            /* DS:0x0724, 0x07ea */
extern i16 mouse_button;                   /* DS:0x0720 */
extern i16 mouse_stroke_going;                      /* DS:0x072a */
extern i16 mouse_changed;                    /* DS:0x072c */
extern i16 mouse_stop_stroke;                 /* DS:0x0730 */
extern i16 mouse_any_stroke;                    /* DS:0x0732 */
extern i16 automate;                      /* DS:0x008a */
extern void text_close(void);
extern i16 keys_any(void);
extern i16 keys_get(void);
extern void mouse_init_cycle(void);
extern void check_music(void);
extern void mouse_begin_cycle(i16 a);
extern void mouse_end_cycle(i16 a, i16 hold);
/* buffer_preserve is (port, int mode, int src, x, y, w, h): -8L below is mode -8 and src -1 in
   ax:dx. Spelled as the two ints, MSC orders an unrelated addition differently; a modern
   compiler needs the two ints (the port patches this call). */
/* PORT: mode and src as the two ints they are (MSC passed -8L in ax:dx) */
extern i16 buffer_preserve(DosPort *port, i16 mode, i16 src, i16 x, i16 y, i16 w, i16 h);
extern void buffer_restore(DosPort *port, i16 handle, i16 flag, i16 x, i16 y, i16 w, i16 h);
extern void load_main_sprites(void);

/* The popup's event loop: draws it, then polls the mouse and the keyboard until an item is
 * chosen, Escape is pressed or (in autoplay) two seconds pass. Returns the chosen key, -1 for
 * Escape. Check-box popups load their states from and save them to popup_checkbox. */
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
/* popup_init -- 033d:3092, 24 bytes */

DosFont *popup_font = 0;           /* DS:0x1f9e (defined here: lands after popup_draw_edits' literals) */
i16 popup_heap = 0x0c00;             /* DS:0x1fa2 */

void popup_init(DosFont *font, i16 size)
{
    popup_font = font;
    popup_heap = size;
}
/* popup_parse_string -- 033d:30aa, 520 bytes */
/* Expands %STRINGn, %NUMBERn, %HEXn (four digits), %COUNTRY, %YEAR and %% in src into dst. */
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
/* popup_start_box -- 033d:32b2, 1062 bytes */
extern char error_string[];                 /* DS:0x3b32 */
i16 popup_start_ask = 0;                      /* DS:0x2008 (defined here: lands after popup_start_box's literals) */
extern char *popup_start_string;              /* DS:0x5e2a */
extern i16 popup_start_max_len;                 /* DS:0x5ad0 */
extern i16 text_open(char *name, char *key);
extern char *text_get(void);

/* Builds a popup from the @-section `key` of the text file `txt`: the text lines, then the
 * options (or an edit field when popup_start_ask is set), with item `sel` preselected. */
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
/* popup_box -- 033d:36d8, 50 bytes */
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
/* pop_clear -- 033d:370a, 8 bytes */
void pop_clear(void)
{
    popup_checkbox = 0;
}
/* pop_set -- 033d:3712, 42 bytes */
void pop_set(i16 bit, i16 on)
{
    bit--;
    if (on)
        popup_checkbox |= 1 << bit;
    else
        popup_checkbox &= ~(1 << bit);
}
/* pop_get -- 033d:373c, 22 bytes */
i16 pop_get(i16 n)
{
    n -= 1;
    return (1 << n) & popup_checkbox;
}
/* pop -- 033d:3752, 14 bytes */
extern char game_text_name[];             /* DS:0x0084 */

i16 pop(char *key)
{
    return popup_box(game_text_name, key, 0);
}
/* pop2 -- 033d:3760, 14 bytes */
i16 pop2(char *key, i16 sel)
{
    return popup_box(game_text_name, key, sel);
}
/* popi -- 033d:376e, 24 bytes */
i16 popi(char *key, i16 tribe)
{
    popup_indian = tribe;
    return popup_box(game_text_name, key, 0);
}
/* pop2i -- 033d:3786, 18 bytes */
i16 pop2i(char *key, i16 sel, i16 tribe)
{
    popup_indian = tribe;
    return popup_box(game_text_name, key, sel);
}
/* popk -- 033d:3798, 24 bytes */
i16 popk(char *key)
{
    popup_indian = 8;
    return popup_box(game_text_name, key, 0);
}
/* popm -- 033d:37b0, 24 bytes */
i16 popm(char *key, i16 dude)
{
    popup_messenger = dude;
    return popup_box(game_text_name, key, 0);
}
/* pop2m -- 033d:37c8, 18 bytes */
i16 pop2m(char *key, i16 sel, i16 dude)
{
    popup_messenger = dude;
    return popup_box(game_text_name, key, sel);
}
/* popf -- 033d:37da, 24 bytes */
i16 popf(char *key, i16 nation)
{
    popup_foreign = nation;
    return popup_box(game_text_name, key, 0);
}
/* pop2f -- 033d:37f2, 18 bytes */
i16 pop2f(char *key, i16 sel, i16 nation)
{
    popup_foreign = nation;
    return popup_box(game_text_name, key, sel);
}
/* popup_off -- 033d:3804, 6 bytes */
/* The next popup is drawn without a frame (0x10) and with flag 8. */
void popup_off(void)
{
    popup_flags |= 0x18;
}
/* popup_ask -- 033d:380a, 76 bytes */
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
/* popup_ask_number -- 033d:3856, 66 bytes */
extern i16 popup_number;                  /* DS:0x5ad4 */

i16 popup_ask_number(char *txt, char *key, i16 dflt)
{
    char buf[0x14];
    i16 result;

    msc_itoa(dflt, buf, 10);
    result = popup_ask(txt, key, buf, 5);
    popup_number = msc_atoi(popup_text_buffer);
    return result;
}
/* popup_read_colors -- 033d:3898, 272 bytes */
/* Reads the popup colours from TEXTCOLR: icon n of the sheet is drawn into a one-pixel port
 * and that pixel is the colour. */
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

/* The popup module's uninitialised data (communals, placed by the linker in BSS). A
 * definition after the last function moves no symbol position. */
char popup_text_buffer[80];               /* DS:0x4b64 */
i16 king_frame;                           /* DS:0x4e7e */
i32 king_clock;                          /* DS:0x4e86 */
i16 popup_start_max_len;                  /* DS:0x5ad0 */
i16 popup_number;                         /* DS:0x5ad4 */
char *popup_start_string;                 /* DS:0x5e2a */
char popup_subst_string[5][0x40];         /* DS:0x634e: 320 bytes */
i32 popup_subst_number[9];               /* DS:0x698a: 38 bytes to the next public; 9 longs */

