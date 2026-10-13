/* From mapedit-decomp src/madsdev/pal_4 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pal_4: MAPEDIT.EXE segment 12b0 (pal_4.c), file 0x14100-0x1427e, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pal_interface 0x14100
 * fn pal_white     0x141ca
 * fn pal_grey      0x14200
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 * WHAT IS LEFT:
 *   - pal_interface (202 bytes, ours 206-212): the level select is the branchless
 *     `cmp dx,1; sbb ax,ax; and al,0xeb; add ax,0x3f` (= k ? 0x3f : 0x2a); every ?:/arithmetic
 *     spelling under "elg" gives a branch or an imul, and only plain/"g" give the sbb form
 *     (but then the loop counters leave their registers). The rest is slot order (base at -6,
 *     r at -8, level never spilled to -0xc) and the chained `base = k = lo2 = 0` init.
 *   - pal_grey (126 bytes, ours 116-128): the roles are decoded (level = lo; step = hi - lo;
 *     n1 = count - 1; left = count counts down in memory; a Bresenham accumulator) but the
 *     allocator puts count in si and p in cx where shipped has count in cx (read from its home)
 *     and p in si, spilled to [bp-6] around the inner loop; an index form (`pal[(start+i)*3]`,
 *     116 bytes) and a named-local form (128) bracket it.
 *
 * STOCK PALETTE ENTRIES (madsdev pal_4.c). A palette is 256 RGB byte triples (0..0x3f).
 *   - pal_interface(pal) (_fastcall) fills entries 0..15 with the 16 interface colours:
 *     two blocks of eight (low level 0 then 0x15; high level 0x2a then 0x3f), each block the
 *     eight r/g/b on-off combinations, then sets entry 6's green to 0x15 (the brown).
 *   - pal_white(pal) (_fastcall) fills entries 0..3 with four greys (0, 0x15, 0x2a, 0x3f).
 *   - pal_grey(start, count, pal, lo, hi) (_fastcall) fills `count` entries from `start`
 *     with a grey ramp from `lo` towards `hi` (lo is the LAST stack parameter, at [bp+8]),
 *     a Bresenham accumulator picking when the level steps.
 */
/* pal_interface -- 12b0:0000, 202 bytes */
void pal_interface(char *pal)
{
    i16 idx;
    i16 g;
    i16 base;
    i16 r;
    i16 lo;
    i16 level;
    i16 b;
    i16 lo2;
    i16 k;

    base = 0;
    for (k = 0, lo2 = 0; lo2 < 0x2a; k++, lo2 += 0x15) {
        lo = lo2;
        level = k < 1 ? 0x2a : 0x3f;
        for (r = 0; r < 2; r++) {
            for (g = 0; g < 2; g++) {
                for (b = 0; b < 2; b++) {
                    idx = base + b;
                    pal[idx * 3] = r ? level : lo;
                    pal[idx * 3 + 1] = g ? level : lo;
                    pal[idx * 3 + 2] = b ? level : lo;
                }
                base += 2;
            }
        }
    }
    pal[0x13] = 0x15;
}
/* pal_white -- 12b0:00ca, 54 bytes */
void pal_white(char *pal)
{
    char levels[4];
    i16 i;

    levels[0] = 0;
    levels[1] = 0x15;
    levels[2] = 0x2a;
    levels[3] = 0x3f;
    for (i = 0; i < 4; i++)
        pal[i * 3 + 2] = pal[i * 3 + 1] = pal[i * 3] = levels[i];
}
/* pal_grey -- 12b0:0100, 126 bytes */
void pal_grey(i16 start, i16 count, char *pal, i16 lo, i16 hi)
{
    i16 acc;
    i16 level;
    char *saved;
    i16 left;
    i16 step;
    i16 n1;
    char *p;

    acc = 0;
    level = lo;
    step = hi - lo;
    if (count > 0) {
        p = pal + start * 3;
        n1 = count - 1;
        left = count;
        do {
            p[2] = p[1] = p[0] = level;
            if (count > 1) {
                acc += step;
                while ((u16)acc >= (u16)n1) {
                    level++;
                    acc += 1 - count;
                }
            }
            p += 3;
        } while (--left);
    }
}

