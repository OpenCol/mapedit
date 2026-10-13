/* From mapedit-decomp src/madsdev/buffer_z (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_z: MAPEDIT.EXE segment 0cb9 (buffer_z.c), file 0xe190-0xe294, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_tile  0x0e190
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bf5).
 *
 * TILING A PORT INTO A RECTANGLE (root code, frame 0bf5; "eg"). buffer_tile(dst, src, x,
 * y, w, h, sx, sy) fills the rectangle (x, y, w, h) of dst with src repeated as a pattern,
 * the pattern's origin at (sx, sy) (the menu backdrop under colour 7, the side panel). Each
 * piece is a buffer_rect_copy_2.
 *
 * `if (cy < bottom9 && (unsigned)right >= 0)` (an always-true term that compiles
 * to nothing, docs/msc6-codegen.md "uses that compile to nothing") puts x + w in si and stores it after the
 * entry test, as shipped: 21 -> 14 bytes. What is left is three frame slots rotated (ours tw3
 * bp-0xc, bottom9 bp-0xa, right bp-0xe; shipped tw3 bp-0xe, bottom9 bp-0xc, right bp-0xa).
 * WITHOUT the term the three slots are already shipped's (right bp-0xa, bottom9
 * -0xc, th2 -0xe); the term pushes right below th2 and bottom9. The slot order is a use weight
 * (replacing th2's clip use with src.h puts right above th2), but folded uses do not count
 * for it (terms in the inner loop test, two terms, terms in the early returns: slots unmoved).
 * Also no effect or worse: the term on x, w or x + w instead of right (x + w puts x in di,
 * 208), the def inside the test as a comma (`if (right = x + w, cy < bottom9)`, 26), cy/bottom9
 * assigned inside the test (21-23), an if(0)/return fold in the entry block (21-136), a dead
 * `right = 0;` first, `right = next = x + w`, `right = x; right += w;`, block-scoped right or
 * bottom9, every clip ?: spelling, and pragmas a/c/w/z/p/n/s/t/l crossed with term/no term.
 * (second pass), still 14. So shipped's right has one reference more than ours, or its si value
 * is not right's web. Neither was found: a second folded term on right in the if (`<=
 * 0xffffu`, `!(< 0)`, `(right + 0)`, `>= 0u`, `||` forms) merges with the first (slot and
 * bytes unchanged); `switch (right) { default: ... }` before the do sinks the store exactly
 * like the term (14) and demotes the slot the same way; `if (0) right = 0;` anywhere, `if
 * (right);`, `(void)right`, `right += 0`, `right = right` are dropped (21); `while
 * ((unsigned)right < 0) ;` anywhere enregisters right in the loops (87-103); a real extra use
 * (`pad_0 = right;`) in the inner loop leaves the term version's slot alone, while a real
 * extra use of th2 at depth 0 or in a loop moves th2 up (so the model is from th2's side). A
 * temp then `right = tmp` at the loop entry (tmp = next, i, oxc, u27, u18, with/without the
 * def first) stores the temp or recomputes x + w at the entry (24-144); `right = x + w` written
 * twice (second inside the if) recomputes it at the entry (24); `x + w` as a CSE at all four
 * sites with no right (174); `while`/`for` outer loops are not rotated under "eg" (153-169);
 * an inner `for (i = x; i < right; i = next)` 71-79; pragma subsets of e/g/l with or
 * without the term 204-237, "egt"/"egz" equal to "eg".
 */



extern i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w,
                                  i16 h);

/* buffer_tile -- 0cb9:0000, 260 bytes */
void buffer_tile(DosPort dst, DosPort src, i16 x, i16 y, i16 w, i16 h, i16 sx, i16 sy)
{
    i16 tw3;
    i16 th2;
    i16 next;
    i16 i;
    i16 oxc;
    i16 oy4;
    i16 cy;
    i16 u27;
    i16 u18;
    i16 ox7;
    i16 bottom9;
    i16 right;

    th2 = src.h;
    tw3 = src.w;
    if (tw3 == 0)
        return;
    if (th2 == 0)
        return;
    next = x - sx > 0 ? x - sx : ~(x - sx) + 1;
    ox7 = next % tw3;
    next = y - sy > 0 ? y - sy : ~(y - sy) + 1;
    oy4 = next % th2;
    u27 = x + w;
    bottom9 = y + h;
    cy = y;
    if (cy < bottom9 && (u16)u27 >= 0) {
        right = u27;
        do {
            oxc = ox7;
            i = x;
            if (i < right) {
                do {
                    next = i - oxc + tw3;
                    buffer_rect_copy_2(oxc, oy4, i, src, dst, cy, (next > right ? right : next) - i,
                                 (cy - oy4 + th2 > bottom9 ? bottom9 : cy - oy4 + th2) - cy);
                    i = next;
                    oxc = 0;
                } while (i < right);
            }
            cy += th2 - oy4;
            oy4 = 0;
        } while (cy < bottom9);
    }
}

