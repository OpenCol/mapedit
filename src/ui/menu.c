/* From mapedit-decomp src/editor/menu (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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
/* MODULE menu: MAPEDIT.EXE segment 06d7 (menu.obj), file 0x8378-0x9a20, 24 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn menu_font_height      0x08378
 * fn menu_set_font         0x0838e
 * fn menu_fill_rect        0x083c2
 * fn menu_string_width     0x08426
 * fn menu_string_write     0x0847c
 * fn menu_get_hotkey       0x085b6
 * fn menu_create           0x0866e
 * fn menu_bar_item         0x087b6
 * fn menu_item             0x087fc
 * fn menu_bar_hide         0x08892
 * fn menu_grey             0x088ca
 * fn menu_release_all_grey 0x088fa
 * fn menu_off              0x0893e
 * fn menu_release_all_off  0x0896e
 * fn menu_add_bar_item     0x089b2
 * fn menu_add_item         0x08b4e
 * fn menu_draw_bar         0x08cb4
 * fn menu_compute_size     0x08ddc
 * fn menu_draw_menu        0x08ef2
 * fn menu_bar_run          0x0910c
 * fn menu_bar_mouse_parse  0x09724
 * fn menu_bar_key_parse    0x097c2
 * fn menu_key_parse        0x09856
 * fn menu_read_colors      0x0991c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl10_44540).
 *
 * THE 1994 MENU BAR: a DOS-only module (Windows uses real menus, so nothing here has a twin).
 * Its nearest relative is overlay 23's popup code: the same text style (spacing, colour,
 * highlight colour, ~hotkey colour, font), the same ~ hotkey parser, the same arena-backed
 * allocation (mem_get + heap_declare). A menu holds a list of submenus (the bar's
 * titles), each a list of items.
 *
 * FLAGS: /Oa instead of /Ow, plus /Oe /Ol /Og: `#pragma optimize("w", off)` then
 * ("aelg", on). Signs: registers by lifetime, DS borrowed as a segment register in list walks
 * (lds ... mov ds,DGROUP), es:di kept across a store through the same pointer (/Oa), and
 * `_fstrcpy(p->text = heap_get(..), name)` pushing `name` BEFORE the nested call (only /Ow
 * hoists the call). menu_bar_run is past MSC's C4203 limit, so it compiles with every local in
 * memory, as shipped.
 *
 * Module data: DS:0x149c-0x14ba are this module's initialised colour words (7,7,7,7,8,8,8,8,0,
 * 0,8,0xf,0,8,0xf, and menu_tile = 0 the backdrop port), followed by "MENUCOLR.SS" at 0x14bc.
 * They are declared extern here; menu_read_colors refills them from MENUCOLR.SS. Finding:
 * its seventh colour is stored to menu_bar_font_main_color a second time (menu_font_grey_color = menu_bar_font_main_color = pix) where menu_bar_font_grey_color
 * looks intended, so the bar's highlight colour (menu_bar_font_grey_color) keeps its default 8.
 *
 *  The history that follows is kept for the levers it
 * records.
 * The length comes from two things together: `dp = m ? m : 0`, whose second definition in
 * another basic block is the only thing that stops MSC merging the pointer copies (324
 * bytes), and `Menu far * volatile result`, which adds the last 4.
 *
 *   - menu_create (238 bytes differ, right length). Shipped keeps three copies of the new menu pointer: m
 *     in di:[bp-0xe], one copy in memory ([bp-0x14]) for the arena call and the first stores,
 *     and dp in si:[bp-2] (spilled around each menu_set_font) for the rest and the result,
 *     with `result` itself in memory. Every spelling tried (copies, casts, long m, void/char
 *     m, statement orders, register, pragmas e/g/l/c/w) copy-propagates them into one.
 *     The merge is block-local: a dp whose definition sits in another basic block survives
 *     (`dp = m ? m : 0`, or `dp = m` above the test) and then gets the shipped allocation
 *     (m di:[bp-0xe], dp si:[bp-2], result in memory, 324 bytes), but both add code shipped
 *     lacks. Chained/embedded copies, long/huge m, loops, goto and labels all still merge.
 * shipped frame read off the bytes (enter 0x20; ours is 0x14, and the
 *     12 missing bytes ARE the difference): result at [bp-0xc]/[bp-0xa], m in di with its
 *     segment at [bp-0xe], dp in si with its segment at [bp-0x2], a third copy at
 *     [bp-0x14]/[bp-0x12] that carries the arena call (`add cx,0x3c`, and `mov ax,[bp-0x14];
 *     add ax,0x4e` for the end pointer) and every field store up to +0xc via `les bx`, and
 *     two spill pairs, [bp-0x1c]/[bp-0x1a] and [bp-0x20]/[bp-0x1e], written from si:es right
 *     before each menu_set_font and read back after; `result = dp` at the end reads the
 *     SECOND spill, not dp. So the copies to chase are m -> (cx:bx, spilled to [bp-0x14])
 *     -> dp, with dp then living in si:es across both calls.
 *     `dp = m ? m : 0` gets closest at 324 bytes of 328 -- but it adds a null test the
 *     shipped code does not have, so it is 4 bytes short somewhere else as well.
 * merged away: routing the allocation through the unused `a`
 *     (`a = mem_get(..); m = a;`, with `dp = a` or `dp = m`) and hoisting the arena's end
 *     pointer into the unused `e` -- all four stay at 306 bytes, byte for byte as now.
 *     Turning the module's own flags off for this function does not help either:
 *     `#pragma optimize("a"|"al"|"ag", off)` leaves 306 bytes (218 differ, our best), and
 *     "ae"/"ael"/"aelg" off drop to 298-302. `volatile` on the pointers overshoots
 *     (dp 363, dp+m 382, m 312) while `volatile result` goes the wrong way (304).
 *     Nor do these keep the copies apart: `dp = (Menu far *)(long)m`, a long temp between
 *     them, `(char huge *)m`, `(Menu far *)((unsigned long)m + 0)`, a separate `char far *e`
 *     or `Menu far *a` carrying one of the two arena-call arguments, taking the end pointer
 *     from m instead of dp, and `register` on m or dp (ignored under /Oe). The one element
 *     read that helps at all is any of the five colour globals (menu_bar_background_color_0, menu_bar_background_color_1, menu_bar_font_main_color,
 *     menu_bar_font_grey_color, menu_bar_font_hilite_color) as `(&g)[0]`: 222 bytes differ instead of 218, same length.
 * against the CURRENT 328-byte version: that same element read --
 *     any one of the five, or two of them -- takes 238 differing bytes down to 192, by far
 *     the best byte count this function has had. It costs 2 of the length (326), and those
 *     2 bytes are worth four MATCHes (menu_draw_bar, menu_compute_size, menu_bar_mouse_parse,
 *     menu_draw_menu all match only because menu_create is exactly 328), so it is NOT taken.
 *     Nothing puts the 2 bytes back: unused locals of any type, `volatile` on a, e or an
 *     extra scalar, and six spellings of the null test (`m == 0 ? 0 : m`, `m != 0 ? m : 0`,
 *     `(long)m ? m : 0`, and the if/else forms both ways round) are all 326 to the byte.
 *     And the ternary is still the only thing that keeps dp off m: with the element read
 *     applied, `dp = m + 0`, `(Menu far *)((char far *)m + 0)` and `&m[0]` all merge again
 *     (300-304), and so does a volatile view of the pointer itself,
 *     `dp = *(Menu far * volatile *)&m` (300). If someone finds the 2 bytes, this function
 *     drops to 192 at a stroke.
 * keeps the copies apart (none adopted, all longer in differing bytes):
 *     the merge is /Oe's, not /Og's -- `#pragma optimize("g"|"gl"|"a"|"ag", off)` around
 *     `p = dp = m;` still gives 304. A copy defined IN THE CONDITION survives:
 *     `if ((dp = m) != 0) { p = m; <arena call and the stores up to +0xc via p>; <rest via
 *     dp> }` keeps all three pointers -- p lands in memory at [bp-0x14] carrying the arena
 *     call and the first stores via `les bx`, and after the first menu_set_font the
 *     title_col pair goes through a spill (`les bx,[bp-0x18]`), the shipped shape -- 324
 *     bytes, 247 differ. Left: the dp copy sits before the test (shipped: after it), m/dp
 *     have si/di swapped, and n_subs' zero store is delayed into the call's pushes.
 *     `dp = &*m` also breaks the copy inside the block (318), but marks m address-taken:
 *     m goes to memory and `size` takes di. Merged as before (304/306): copies in nested
 *     blocks, labels between copy and use, copies chained onto the mem_get call, copies
 *     before the test, `dp = 0` first, `(dp = m, 1)` in a `&&`, `(a = m) != 0` on an unused
 *     local, `&m->f0`, `m + (size & 0)`, `(void far *)` casts, array/struct holders (288). An inner `if (dp != 0)` after `dp = m` gives shipped's exact first
 *     four instructions (`mov ax,[bp-0xe]; mov cx,di; mov bx,ax; mov si,cx`) but MSC emits
 *     the test (316): it folds no test that is not a literal constant, not even a repeated
 *     `m != 0`. Copies above the test (as a statement, in the condition, a (void far *)
 *     cast, `p` and `dp` both, or the alloc through result) stay 318-324. Copies embedded in the arena call's arguments (`(dp = m)->arena`,
 *     `(p = dp = m)->arena`, `(char far *)(p = m) + 0x4e`), one pointer per store group
 *     (p/q/r = m or = dp), `dp = m` repeated per group, `dp = dp` after a call, the inverted
 *     if with an empty then-arm (`if (m == 0) ; else {...}`, `if (!m) ; else`, `{} else`,
 *     `goto out`), a non-volatile result, and the ternary with a separate p for the first
 *     group (p = m / p = dp, 348) all merge to the same 304 bytes or worse.
 *   - menu_draw_menu's own 12 bytes were the two addend orders in its buffer_draw_box call
 *     (`h + y - 1` loading h first, `w + x - 1` loading x first). buffer_draw_box's FIRST
 *     parameter declared `unsigned` fixes both, and the pad count then moves from 10 to 8
 *     (1..8 and 12..14 all leave the five displacement bytes; 1 breaks menu_bar_run).
 *     The type is what does it -- buffer_draw_box's prototype POSITION and the local declaration
 *     order never moved these addends.
 *   - menu_create NOW.
 *     Three levers, each needed:
 *     (1) four names for the pointer, defined in the order dp, p, q (sources do not matter,
 *     they are all m after propagation: `q = p = dp = m;`), then a basic-block boundary made
 *     by an always-true test that READS the pointer (`(unsigned long)m >= 0`; tests on font,
 *     size, result, `if (1)`, labels, gotos and an outer switch all merge the copies). p
 *     carries the arena end pointer and the first store group, q the arena itself.
 *     (2) `switch (0) { default: <drop_col..frame_col> }` (or `switch (size) { case 0:
 *     default: }`) ends the first CSE temp after title_col2: the title stores still go
 *     through the spill (`les bx,[bp-0x1c]`), the drop group reloads es from dp's [bp-2] and
 *     uses si, and the second menu_set_font gets its own spill, read back for `result` --
 *     the shipped shape. do-while(0), while(1)+break, if(1), goto/labels and an always-true
 *     if around the group are no barrier (or a different one).
 *     (3) two no-code reads of q in the test (`&& (unsigned long)q >= 0` twice): the arena
 *     call's segment then comes from ax (`push ax`) instead of a fresh `mov dx,ax`, and the
 *     end pointer gets shipped's `mov dx,bx`. One read, or reads of m/dp/p/result, do not.
 *     The n_subs volatile view can sit on title_gap, item_gap, item_pad, bar_col or bar_col2
 *     instead (same bytes); without any, 334 bytes.
 *     LEFT: (a) `push ax` where shipped pushes bx (bx is the pair p is stored from); (b) the
 *     frame: ours is enter 0x28 with p -8, dp -12 (si), q -20, result -24, m -28 (di), spills
 *     -36/-40 and three never-touched slots (-4, -16, -32); shipped is enter 0x20 with dp -4,
 *     result -12, m -16, [bp-0x14] -20, spills -28/-32 and two untouched slots (-8, -24).
 *     The /Fa listing names the locals' offsets (`;  p = -8`); unused locals pack into the
 *     holes. Frame-only levers tried, none moved a slot the right way: declaration order and
 *     0-16 pads (padmove: all 153 variants identical), block-scope declarations of p/q/dp,
 *     unused locals, no-code reads of any copy in the test or the tail test, prototype types
 *     of heap_declare. `(void)&p` puts dp at -4 and p at -20 as shipped (frame 0x24, q -12,
 *     result -16, m -24) but makes `p->gap = p->bar_h = 1` re-read bar_h; written through
 *     dp it is 33 bytes. A second family has the first store group through m itself (a CSE
 *     temp spilled at once, like shipped's [bp-0x14]) and four no-code reads of dp plus one of
 *     q: dp -4, m -16 and the spill shape right, but the arena arguments come out as `mov
 *     ax,cx; ... add cx,0x4e` (44 bytes). Also no better: every chain order and a/e source
 *     (only dp, p, q with a=q e=p works), a fifth copy, a/e single-use locals, `m + 1` /
 *     `&m->arena[0x12]` spellings of the end pointer, q copied inside the block, volatile
 *     views on other stores or on result.
 *   - menu_create FRAME. /Fa listings:
 *     the never-touched slots are not unused locals (an unused local packs INTO a hole) but
 *     things the code creates. The slot at -4 appears only with the arena call and the extra
 *     copies (without the call, q/dp move up); each copy kept apart (p, q) costs its own slot
 *     plus one hole, so three copies give 0x28 and two give 0x20. Two-copy shapes with the
 *     right frame size, all longer in bytes: `q = dp = m;` with the first group through q and
 *     the arena call through m -- 0x20, 27 lines off, but the arena and end pointer share one
 *     register CSE (`mov ax,cx; mov dx,bx; add ax,0x3c; ...; mov ax,cx`) where shipped consumes
 *     cx (`add cx,0x3c; push bx; push cx`) and reloads the offset (`mov ax,[bp-0x14]; mov dx,bx`)
 *     -- that reload is a memory variable's register cache, which is why the third copy (q,
 *     enregistered, consumed by the arena) is needed at all. A no-code read of q in the TAIL
 *     test lengthens q's lifetime and moves its slot down (-8 to -16); reads in the inner test
 *     move nothing. `(void)&p` (aliased sorts last) gives dp -4, result -16, p -20 (shipped's),
 *     q -12, m -24, frame 0x24, but makes `p->gap = p->bar_h = 1` re-read bar_h (332; an int
 *     `one`, a cast, or a split store do not avoid it; through dp it is 33 bytes). Also no
 *     help: routing the later stores through m, the arena call before/after the copies outside
 *     the test, every chain order x 0-3 q reads x aliased p. A different family from the NOW entry
 *     below: no p/q copies and no inner test. Only result, m and dp; the arena call and the
 *     first store group go through m itself, so MSC's own global CSE temp of m (cx:bx, spilled
 *     to [bp-0x14] across heap_declare) is shipped's [bp-0x14]: with `dp = m;` in the same
 *     block it gives the shipped `mov cx,di; mov bx,ax; mov si,cx` and `add cx,0x3c; push bx`
 *     at once. Each remaining piece is needed (dropping any one: 3-93 bytes):
 *     (1) `if ((unsigned)size < 0) m = 0;` after the first group: a late-folded redefinition of
 *     m, so the bar/drop stores are not known to equal the CSE and go through dp (si, es from
 *     [bp-2]). Only a def of m works there (m = result, m++, a cast); defs of dp do not.
 *     (2) the drop group in `switch (0) { default: }` (the second spill temp, [bp-0x20]).
 *     (3) `if ((unsigned)size < 0) dp->f0 = 0;` after (1) lifts dp to [bp-4] (si) and m to di;
 *     `if ((unsigned)size < 0) result = 0;` before the return lifts result above m.
 *     (4) `if (((char far *)m + 0x4e) != ((char far *)m + 0x4e)) result = 0;` right after the
 *     first group: the self-comparison folds only AFTER global CSE has made a temp for the end
 *     pointer, and that temp keeps a never-touched slot in creation order between the m temp
 *     and the two spill temps -- shipped's untouched [bp-0x18]. That moved the spills from
 *     -0x18/-0x1c to -0x1c/-0x20 (the last 8 bytes).
 *     LEARNED: under "elg" the frame is named locals (by weight) then compiler temps in creation
 *     order (CSE temps and the si:es spill temps of each call alike). A named local, even an
 *     aliased or block-scoped one, never sorts below a temp, so an untouched slot among the
 *     temps is a temp: make one with an always-false self-comparison of the expression; dead
 *     late-folded blocks, `(void)expr`, a folded `&&` term and switch(0) wrappers create none.
 *     An unused aliased far pointer with a dead def (`(void)&x; if ((unsigned)size < 0) x = 0;`)
 *     adds a named hole after m (4 bytes left there). 10 pads now (the function lost two locals).
 *   - menu_create NOW: two copies `p = dp = m;`, a late-folded
 *     `if ((unsigned)size < 0) p = 0;` before an always-true `(unsigned long)m >= 0` test,
 *     `q = m;` inside it, the arena call alone in `switch (0) { default: }`, and a folded
 *     `&& (unsigned long)p >= 0` on the dtor test; 5 pads after (0-4 and 6-16 break
 *     menu_add_bar_item, menu_draw_bar, menu_draw_menu or menu_bar_run). Left: the slot order --
 *     ours result -0x14, m -0x16 (segment), the store copy -0x10; shipped result -0xc,
 *     m -0xe, copy -0x14 -- plus `mov si,di` for `mov si,cx` and `push ax` for `push bx`.
 *   - menu_create DEAD BLOCKS. Late-folded
 *     `if ((unsigned)size < 0) stmt;` blocks (docs/msc6-codegen.md) DO move slots here, but only blocks
 *     outside the inner if keep the code: inside it each one is a block boundary (316-336
 *     bytes). On the kept version single dead defs/reads of p, q, dp, m or result at the top,
 *     between the copies and the test, before the tail test or before the return move
 *     dp/p/q/result/m around (e.g. a dead `q->f0 = 0;` read before the return: q -24) but
 *     never shrink 0x28, and 319 combinations of them reach 24 masked bytes only by
 *     coincidence in a 0x2c-0x30 frame. Best frame: the two-copy shape (`q = dp = m;`, the
 *     first group through q, the arena call through m) plus `if ((unsigned)size < 0) q = 0;`
 *     at the top and `if ((unsigned)size < 0) q->f0 = 0;` before the return gives enter 0x20
 *     with dp -4, m's segment at -0xe and the spills at -0x1c/-0x20 exactly as shipped (q -16,
 *     result -20, untouched -8/-12), 328 bytes -- but its arena code is the shared CSE
 *     (`mov si,di ... mov ax,di; add ax,0x3c ... add cx,0x4e`), 237 masked bytes. Dead reads
 *     of result (0-8) and m (0-6), dead defs of either, do not lift them above q; a dead
 *     def of q with the arena call through q gives 318-338.
 *   - menu_create LEAD:
 *     four pointer copies chained in an always-true test plus a volatile view on ONE field
 *     store reach 115 differing bytes, but 320 of the 328 (four functions here need 328):
 *       Menu far *result; Menu far *m; Menu far *dp; Menu far *p; void far *a; void far *e;
 *       Menu far *q;   ...   if (m != 0) { if ((unsigned long)(q = p = dp = m) >= 0) {
 *       e = (char far *)p + 0x4e; a = q->arena; heap_declare(0x28, a, e, (long)size);
 *       p->subs = 0; *(volatile int far *)&p->n_subs = 0; ... (stores via p) ...
 *     (the full variant is "Z3 vol n_subs" in that pass's v21.py; reconstruct from this note).
 *     What is left in it: the a/e argument evaluation (`mov dx,ax; push dx` vs shipped
 *     `push bx; ... mov dx,bx`), and shipped keeps the pointer in si with es reloaded from
 *     [bp-2] for the second store group (0x12..0x1e, lea [si+0x2c]) and spills si:es to
 *     [bp-0x20] around the call, where Z3 uses es:bx -- the 8 missing bytes. The item-hit loop leaves by
 *     `goto items_done` (a `break` or `if (it) ... else break` is hoisted into the loop test,
 *     which turns the loop top-tested, ends the not-found arm in a jump and lets MSC park the
 *     whole arm after the key dispatch); the same for the hotkey loop (`goto hot_done`). The
 *     click test's drag/height half is a goto-parked block after the `m->f0 = sel->id` arm's
 *     jump, reached by `if (sel != 0) ; else goto click_check;` (a conditional goto gets the
 *     block's first test threaded into it). `redraw7 = found = 1` stores found first.
 */

#include "msc.h"
#include "msc.h"


/* A text style: the extra spacing after each character, the plain colour, the highlighted
 * colour, the ~hotkey colour, the font. 0xc bytes. */
/* A menu item: flags (1 disabled, 2 hidden), its hotkey, its id, its text, the list links. */
/* A submenu (a title on the bar): its x on the bar, its width, the drop-down's item height,
 * hotkey, id, flags (1 hidden), title, owner, links, items. 0x22 bytes. */
/* menu_font_height -- 06d7:0008, 22 bytes */
i16 menu_font_height(DosFont *f)
{
    i16 r;

    r = f->height;
    if (r == 6)
        r--;
    return r;
}
/* menu_set_font -- 06d7:001e, 52 bytes */
void menu_set_font(MenuStyle *st, DosFont *font, i16 spacing, i16 color, i16 hi, i16 alt)
{
    st->spacing = spacing;
    st->color = color;
    st->hi = hi;
    st->alt = alt;
    st->font = font;
}
/* menu_fill_rect -- 06d7:0052, 100 bytes */

/* buffer_rect_fill, except that colour 7 restores the backdrop picture when there is one. */
void menu_fill_rect(i16 x, i16 y, i16 w, DosPort port, i16 h, i16 a, i16 b, i16 c,
                                   char color, char color2, i16 d, i16 e)
{
    if (menu_tile != 0 && color == 7) {
        buffer_tile(port, *menu_tile, x, y, w, h, a, b);
        return;
    }
    buffer_rect_fill(x, y, w, port, h, color);
}
/* menu_string_width -- 06d7:00b6, 86 bytes */

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
/* menu_string_write -- 06d7:010c, 314 bytes */

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
/* menu_get_hotkey -- 06d7:0246, 184 bytes */
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
/* menu_create -- 06d7:02fe, 328 bytes */
/* The menu colours and the backdrop port, defined here (the 16 land at DS:0x0614-0x0634 in
 * this order, ahead of menu_read_colors' "MENUCOLR.SS": MSC emits a global after the literals
 * of the function that follows its definition, and no function before that one has any). */
i16 menu_bar_background_color_0 = 7;      /* DS:0x0614 */
i16 menu_bar_background_color_1 = 7;      /* DS:0x0616 */
i16 menu_background_color_0 = 7;          /* DS:0x0618 */
i16 menu_background_color_1 = 7;          /* DS:0x061a */
i16 menu_select_color_0 = 8;              /* DS:0x061c */
i16 menu_select_color_1 = 8;              /* DS:0x061e */
i16 menu_bar_select_color_0 = 8;          /* DS:0x0620 */
i16 menu_bar_select_color_1 = 8;          /* DS:0x0622 */
i16 menu_border_color = 0;                /* DS:0x0624 */
i16 menu_font_main_color = 0;             /* DS:0x0626 */
i16 menu_font_grey_color = 8;             /* DS:0x0628 */
i16 menu_font_hilite_color = 0xf;         /* DS:0x062a */
i16 menu_bar_font_main_color = 0;         /* DS:0x062c */
i16 menu_bar_font_grey_color = 8;         /* DS:0x062e */
i16 menu_bar_font_hilite_color = 0xf;     /* DS:0x0630 */
DosPort *menu_tile = 0;                   /* DS:0x0632, the menu's backdrop port (declared above) */

Menu *menu_create(i16 size, DosFont *font)
{
    Menu *result;
    Menu *m;
    Menu *dp;

    result = 0;
    /* PORT: the arena twice the 1994 size -- its records hold pointers, twice as wide now
       (and /Zp2 adds no padding), so none is more than twice what it was */
    m = mem_get((i32)(2 * (i32)size + sizeof(Menu)));   /* PORT: 0x4e bytes in 1994 */
    if (m != 0) {
        dp = m;
        heap_declare(0x28, m->arena, (char *)m + sizeof(Menu), 2 * (i32)size);   /* PORT */
        m->subs = 0;
        m->n_subs = 0;
        m->title_gap = 0xc;
        m->item_gap = 3;
        m->item_pad = 4;
        m->gap = m->bar_h = 1;
        if (((char *)m + sizeof(Menu)) != ((char *)m + sizeof(Menu))) result = 0;   /* PORT */
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
/* menu_bar_item -- 06d7:0446, 70 bytes */
/* 10 pads: the later functions' mod-17 symbol positions (every other count 0-16 breaks
 * menu_add_bar_item, menu_draw_bar, menu_draw_menu, menu_bar_run or menu_bar_mouse_parse) */

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
/* menu_item -- 06d7:048c, 150 bytes */
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
/* menu_bar_hide -- 06d7:0522, 56 bytes */
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
/* menu_grey -- 06d7:055a, 48 bytes */
void menu_grey(Menu *m, i16 id, i16 on)
{
    MenuEntry *it;

    it = menu_item(m, id);
    if (on)
        it->flags |= 1;
    else
        it->flags &= ~1;
}
/* menu_release_all_grey -- 06d7:058a, 68 bytes */
void menu_release_all_grey(Menu *m)
{
    MenuSub *sub;
    MenuEntry *it;

    for (sub = m->subs; sub != 0; sub = sub->next)
        for (it = sub->items; it != 0; it = it->next)
            it->flags &= ~1;
}
/* menu_off -- 06d7:05ce, 48 bytes */
void menu_off(Menu *m, i16 id, i16 on)
{
    MenuEntry *it;

    it = menu_item(m, id);
    if (on)
        it->flags |= 2;
    else
        it->flags &= ~2;
}
/* menu_release_all_off -- 06d7:05fe, 68 bytes */
void menu_release_all_off(Menu *m)
{
    MenuSub *sub;
    MenuEntry *it;

    for (sub = m->subs; sub != 0; sub = sub->next)
        for (it = sub->items; it != 0; it = it->next)
            it->flags &= ~2;
}
/* menu_add_bar_item -- 06d7:0642, 412 bytes */

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
    s = heap_get((i32)sizeof(MenuSub), m->arena);   /* PORT: 0x22 bytes in 1994 */
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
/* menu_add_item -- 06d7:07de, 358 bytes */
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
        it = heap_get((i32)sizeof(MenuEntry), m->arena);   /* PORT: 0x16 bytes in 1994 */
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
/* menu_draw_bar -- 06d7:0944, 296 bytes */

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
/* menu_compute_size -- 06d7:0a6c, 278 bytes */

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
/* menu_draw_menu -- 06d7:0b82, 538 bytes */


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
/* menu_bar_run -- 06d7:0d9c, 1560 bytes */
/* buffer_preserve is (port, int mode, int src, x, y, w, h): -8L below is mode -8 and src -1 in
   ax:dx. Spelled as the two ints, MSC orders an unrelated addition differently; a modern
   compiler needs the two ints (the port patches this call). */
/* PORT: mode and src as the two ints they are (MSC passed -8L in ax:dx) */

/* The drop-down's event loop: the mouse picks an item or moves to another title, the arrow
 * keys (and keypad 8/2) move the bar, left/right switch submenu, Enter takes the bar, Esc
 * cancels, a letter picks its ~hotkey item, and releasing Alt closes the menu. The chosen
 * item's id (0 for none) is left in the menu's f0. */
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
/* menu_bar_mouse_parse -- 06d7:13b4, 158 bytes */

/* A mouse press on the bar opens that title's drop-down (when mode is set); 1 if it hit one. */
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
/* menu_bar_key_parse -- 06d7:1452, 148 bytes */

/* An Alt+letter opens the title with that ~hotkey; 1 if one opened. */
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
/* menu_key_parse -- 06d7:14e6, 198 bytes */
/* A key that is some enabled item's hotkey (F-keys) chooses it directly: returns its id. */
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
/* menu_read_colors -- 06d7:15ac, 260 bytes */

/* The menu colours, read from the pixels of MENUCOLR.SS. */
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

