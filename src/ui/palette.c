/* palette.c -- the palette: allocating colours, shadows, writing and reading the DAC, colour cycling, the screen shake. */
#include <string.h>

#include "msc.h"
#include "host.h"
#include "error.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mcga.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sort.h"
#include "sprite.h"
#include "timer.h"
#include "units.h"
#include "w16types.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE pal_1: MAPEDIT.EXE segment 1074 (pal_1.c), file 0x11d4e-0x12742, 13 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pal_call_manager    0x11d4e   (static_1074_000e: near, no public)
 * fn pal_init            0x11d6c
 * fn pal_lock            0x11e2c
 * fn pal_unlock          0x11e7a
 * fn pal_deallocate      0x11e9a
 * fn pal_compact         0x11f1a
 * fn pal_get_hash        0x11fc8
 * fn pal_shadow_sort     0x11fee
 * fn pal_init_shadow     0x12072
 * fn pal_activate_shadow 0x120d2
 * fn pal_find_free_flag  0x120e6   (static_1074_03a6: near, no public)
 * fn pal_count_free      0x12120   (static_1074_03e0: near, no public)
 * fn pal_allocate        0x12156
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 *  What it took: color_status loops indexed by `i`
 * (the pointer IV leaves a dead load of i's home, `mov cx,[bp-2]`, in pal_lock and
 * pal_deallocate); the byte flags as `|= 0x80000000L` / `&= 0x7fffffffL` (MSC narrows the
 * long masks to `or/and byte [bx+3]`); pal_compact uses ONE `i` for both loops and one `bit`
 * for both shifts (slot weights); pal_shadow_sort's keys[3] (the frame) and an indexed loop
 * (the `lea ax,[si+2]` IV); pal_find_free_flag's `goto out` (two epilogues, r kept in di across
 * the call). pal_call_manager takes the callback as two words (`off | seg` loads the offset
 * first; a long or far pointer loads the segment first) with 7 pads.
 * WHAT IS LEFT:
 *   - pal_call_manager: 2 bytes, `code` stored through bx (ours dx/ax); the late-folded
 *     `if ((unsigned)off < 0) code = 0;` moves it to dx only.
 *   - pal_init: the two fill copies are `rep movsw` (forward, and `std` backward from
 *     color_status[254].hi) with no entry test; pointer loops convert to movsw but keep a
 *     `dec/jz` entry test, memcpy's intrinsic adds the odd-byte movsb; pal_deallocate's near
 *     call displacement follows pal_init's length.
 *   - pal_init_shadow: register names only (shipped sh in ds:di and i in bx; ours sh in
 *     es:bx and i in di). Left: the i2 loop head
 *     is parked after the outer loop test (an out-of-line block), the `while (!found)` shadow
 *     loops' layout, and operand orders of the limit min/max statements.
 *
 * THE PALETTE MANAGER (madsdev pal_1.c; "elg"). master_palette (DS:0x5b20) is the 256 RGB
 * triples the hardware shows; color_status (DS:0x6572) is a long per entry: bit 0 = a stock
 * colour (reserved at init), bit 1 = allocated, bits 2.. = the "flags" (allocation groups,
 * one bit each; flag_used[32] says which groups are live) and bit 31 = locked.
 *   - pal_call_manager(fn, code) (near) runs the pal_manager_update callback with
 *     pal_manager_active set to the code, when one is installed.
 *   - pal_init(bottom, top) (_fastcall) clears the status, loads the 16 interface colours
 *     once (pal_interface), marks `bottom` entries from 0 and `top` entries from 255 as
 *     stock, frees every flag but 0 and 1, and tells the manager (code 1).
 *   - pal_lock / pal_unlock take flag 31 and set or clear bit 31 on every used entry.
 *   - pal_deallocate(flag) (_fastcall) clears a flag's bit from every entry, frees the
 *     entries that were only allocated, releases the flag and tells the manager (code 4).
 *   - pal_compact(flag, n, list) (_fastcall) moves the entries of the n flags in `list`
 *     onto `flag`.
 *   - pal_get_hash(a, b) (_fastcall) is the squared RGB distance of two triples (asm).
 *   - pal_shadow_sort(list, sh): keys the shadow's entries by magic_hash_color and sorts
 *     them (sort_insertion). pal_init_shadow(list, sh) collects up to three entries with
 *     flag 0x10 into the shadow. pal_activate_shadow(sh) installs it as master_shadow.
 *   - pal_find_free_flag (near) returns the first free flag or -10 with error -5;
 *     pal_count_free(&first) (near) counts the free entries and returns the first.
 *   - pal_allocate(flags, sh, list) (_fastcall) assigns palette entries to a request list.
 */

typedef union { i32 l; struct { u8 b0, b1, b2, b3; } b; } ColorStatus;
typedef void (*PalFn)(void);

/* pal_call_manager -- 1074:000e, 30 bytes */
void pal_call_manager(i32 fn, i16 code)
{
    if (fn) {
        pal_manager_active = code;
        (*(PalFn *)&fn)();
        pal_manager_active = 0;
    }
}
/* pal_init -- 1074:002c, 192 bytes */
void pal_init(i16 bottom, i16 top)
{
    i16 i;
    i32 *p;

    palette_low_search_limit = 0;
    palette_high_search_limit = 0x100;
    for (p = color_status; p < color_status + 256; p++)
        *p = 0;
    if (palette_ever_initialized == 0) {
        pal_interface(master_palette);
        palette_ever_initialized = -1;
    }
    if (bottom > 0) {
        color_status[0] = 1;
        for (i = 0; i < (bottom - 1) * 2; i++)
            ((i16 *)color_status)[i + 2] = ((i16 *)color_status)[i];
    }
    if (top > 0) {
        color_status[255] = 1;
        for (i = 0; i < (top - 1) * 2; i++)
            ((i16 *)color_status)[511 - 2 - i] = ((i16 *)color_status)[511 - i];
    }
    for (i = 0; i < 32; i++)
        flag_used[i] = 0;
    flag_used[1] = flag_used[0] = -1;
    palette_reserved_bottom = bottom;
    palette_reserved_top = top;
    pal_manager_colors = palette_locked = 0;
    pal_call_manager(pal_manager_update, 1);
}
/* pal_lock -- 1074:00ec, 78 bytes */
void pal_lock(void)
{
    i16 i;

    if (flag_used[31] != 0 && palette_locked == 0)
        error_report(-5, 2, 3, 0x20L, 1L);
    flag_used[31] = palette_locked = -1;
    for (i = 0; i < 256; i++)
        if (color_status[i] != 0)
            color_status[i] |= 0x80000000L;
}
/* pal_unlock -- 1074:013a, 32 bytes */
void pal_unlock(void)
{
    i16 i;

    if (palette_locked) {
        for (i = 0; i < 256; i++)
            color_status[i] &= 0x7fffffffL;
        /* PORT: flag_used[32] is one past the array: DS:0x52d4, which is map_view_size_x */
        extern i16 map_view_size_x;
        palette_locked = map_view_size_x = 0;
    }
}
/* pal_deallocate -- 1074:015a, 128 bytes */
i16 pal_deallocate(i16 flag)
{
    i32 mask;
    i16 i;

    if (flag < 32 && flag > 0) {
        mask = ~(1L << flag);
        for (i = 0; i < 256; i++) {
            color_status[i] &= mask;
            if (color_status[i] == 2L)
                color_status[i] = 0;
        }
        if (flag_used[flag] != 0) {
            flag_used[flag] = 0;
            pal_call_manager(pal_manager_update, 4);
        }
    }
    return 0;
}
/* pal_compact -- 1074:01da, 174 bytes */
void pal_compact(i16 flag, i16 n, i16 *list)
{
    i32 keep;
    i32 bits;
    i32 bit;
    i16 i;

    keep = -1L;
    bits = 0;
    for (i = 0; i < n; i++) {
        bit = 1L << list[i];
        keep ^= bit;
        bits |= bit;
        flag_used[list[i]] = 0;
    }
    bit = 1L << flag;
    for (i = 0; i < 256; i++) {
        if (bits & color_status[i]) {
            color_status[i] &= keep;
            color_status[i] |= bit;
        }
    }
    flag_used[flag] = -1;
}
/* pal_get_hash -- 1074:0288, 38 bytes */
/* PORT: pal_get_hash is in port/ (inline assembly) */

/* pal_shadow_sort -- 1074:02ae, 132 bytes */
void pal_shadow_sort(Shadow *sh, PalList *list)
{
    i32 keys[3];
    i16 i;

    for (i = 0; i < sh->n; i++)
        keys[i] = magic_hash_color(list->e + sh->idx[i]);
    sort_insertion(sh->n, sh->idx, keys);
}
/* pal_init_shadow -- 1074:0332, 96 bytes */
void pal_init_shadow(Shadow *sh, PalList *list)
{
    i16 i;
    u8 *p;

    i = 0;
    sh->n = 0;
    if (list->n > 0) {
        p = (u8 *)list + 7;
        do {
            if (*p & 0x10) {
                if (sh->n < 3) {
                    sh->idx[sh->n] = i;
                    sh->n++;
                }
            }
            p += 6;
            i++;
        } while (i < list->n);
    }
}
/* pal_activate_shadow -- 1074:0392, 20 bytes */
void pal_activate_shadow(Shadow *sh)
{
    master_shadow = sh;
}
/* pal_find_free_flag -- 1074:03a6, 58 bytes */
i16 pal_find_free_flag(void)
{
    i16 r;
    i16 i;

    r = -10;
    for (i = 0; i < 32; i++) {
        if (flag_used[i] == 0) {
            r = i;
            goto out;
        }
    }
    error_report(-5, 2, 3, 0x20L, 1L);
out:
    return r;
}
/* pal_count_free -- 1074:03e0, 54 bytes */
i16 pal_count_free(i16 *first)
{
    i16 n;
    i16 i;

    *first = -1;
    n = 0;
    for (i = 0; i < 256; i++) {
        if (color_status[i] == 0) {
            n++;
            if (*first < 0)
                *first = i;
        }
    }
    return n;
}
/* pal_allocate -- 1074:0416, 1516 bytes */
typedef struct { u8 r, g, b; } Rgb;
/* 4 pads: the limit compares load the global into ax (`mov ax,[g]; cmp ax,[lo8]`); 0..3 pads
 * compare the other way round (position mod 17). */
i16 pal_allocate(i16 flags, PalList *list, Shadow *sh)
{
    i16 best2;
    i16 mask25;
    i16 nfree3;
    i16 lo8;
    i16 bestidx;
    i16 score9;
    u8 kindsl[0x100];
    i16 exact;
    i16 kk;
    char *orderpj;
    i16 cond11;
    i16 i2k;
    i16 exactshadowk;
    i16 cond2;
    Shadow localsh1;
    u8 orderk[0x100];
    i32 allocl;
    i16 first;
    i16 i;
    i32 mask2l;
    i16 flag;
    i16 fixedval8;
    i16 unused232;
    i16 allocflag8;
    i16 spare2;
    i16 useshadowj;
    u8 *pal;
    i16 maxcol9;
    i16 found1;
    i16 m;
    i32 flagbit4;
    i16 hil;
    char buf[4];
    register i16 c;
    register i16 ei;

    orderpj = buf;
    pal = master_palette;
    maxcol9 = (flags & 0x800) ? 0x100 : 0xfc;
    if (flags & 0x4000) {
        lo8 = 0;
        hil = maxcol9;
    } else {
        lo8 = palette_reserved_bottom;
        hil = min(0x100 - palette_reserved_top, maxcol9);
    }
    lo8 = max(palette_low_search_limit, lo8);
    hil = min(palette_high_search_limit, hil);
    flag = pal_find_free_flag();
    if (flag < 0)
        goto out;
    flagbit4 = 1L << flag;
    exact = flags & 0x8000;
    mask25 = flags & 0x8400;
    useshadowj = (sh != 0) ? 1 : 0;
    exactshadowk = 0;
    if (useshadowj) {
        if (exact != 0 || sh->n == 0)
            useshadowj = 0;
        if (exact != 0 && sh->n != 0)
            exactshadowk = -1;
    }
    if (useshadowj) {
        pal_init_shadow(&localsh1, list);
        pal_shadow_sort(&localsh1, list);
    }
    nfree3 = pal_count_free(&first);
    first = max(lo8, first);
    for (i = 0; i < list->n; i++) {
        orderk[i] = (u8)i;
        kindsl[i] = 0;
        if (!(list->e[i].flags & 0x80))
            kindsl[i] = 0x40;
        if (list->e[i].flags & 0x60)
            kindsl[i] |= 0x20;
    }
    sort_insertion_8(list->n, orderk, kindsl);
    if (flags & 0x4000)
        fixedval8 = -1;
    else
        fixedval8 = -2;
    unused232 = -1;
    for (i2k = 0; i2k < list->n; i2k++) {
        found1 = 0;
        bestidx = -1;
        ei = orderk[i2k];
        if (list->e[ei].flags & 8) {
            found1 = -1;
            bestidx = 0xfd;
        }
        if (useshadowj && (list->e[ei].flags & 0x10)) {
            kk = 0;
            while (!found1) {
                if (kk >= localsh1.n)
                    break;
                if (localsh1.idx[kk] == ei) {
                    found1 = -1;
                    m = min(sh->n - 1, kk);
                    bestidx = sh->idx[m];
                }
                kk++;
            }
        }
        if (exactshadowk && (list->e[ei].flags & 0x10)) {
            kk = 0;
            while (!found1) {
                if (kk >= master_shadow->n)
                    break;
                if (master_shadow->idx[kk] == ei) {
                    found1 = -1;
                    bestidx = kk + 0xf0;
                    msc__fmemcpy(master_palette + bestidx * 3, &list->e[ei].r, 3);
                }
                kk++;
            }
        }
        if (list->e[ei].flags & 0x80)
            mask2l = 0;
        else
            mask2l = 2;
        cond11 = (found1 == 0 && exact == 0) ? 1 : 0;
        if (cond11) {
            if ((list->e[ei].flags & 0x20) ||
                (((flags & 0x2000) || (list->e[ei].flags & 0x40)) && ((flags & 0x1000) || nfree3 == 0)))
                best2 = 0x7fff;
            else
                best2 = 1;
            for (c = lo8; c < hil; c++) {
                if (color_status[c] == 0)
                    continue;
                if ((color_status[c] & 1) && !(flags & 0x4000))
                    continue;
                if (color_status[c] & mask2l)
                    continue;
                if (best2 > 1) {
                    score9 = pal_get_hash(&list->e[ei].r, master_palette + c * 3);
                } else {
                    if (*(i16 *)&list->e[ei].r == *(i16 *)(master_palette + c * 3) &&
                        list->e[ei].b == master_palette[c * 3 + 2])
                        score9 = 0;
                    else
                        score9 = 1;
                }
                if (best2 > score9) {
                    found1 = -1;
                    bestidx = c;
                    best2 = score9;
                }
            }
        }
        if (found1 == 0) {
            cond2 = (!(flags & 0x1000) || (!(list->e[ei].flags & 0x60) && !(flags & 0x2000))) ? 1 : 0;
            if (cond2) {
                c = first;
                while (!found1) {
                    if (c >= hil)
                        break;
                    if (color_status[c] == 0) {
                        nfree3--;
                        first++;
                        found1 = -1;
                        bestidx = c;
                        *(Rgb *)(master_palette + c * 3) = *(Rgb *)&list->e[ei].r;
                    }
                    c++;
                }
            }
        }
        if (found1 == 0)
            goto fail;
        allocflag8 = (exact != 0 && list->e[ei].extra != 0) ? 2 : 0;
        allocl = allocflag8;
        color_status[bestidx] |= allocl | flagbit4;
        list->e[ei].index = (u8)bestidx;
    }
    flag_used[flag] = -1;
    pal_manager_colors = list->n;
    pal_call_manager(pal_manager_update, 2);
    goto out;
fail:
    pal_manager_colors = list->n;
    pal_call_manager(pal_manager_update, 3);
    error_report(-6, 2, 3, (i32)list->n, (i32)i2k);
    flag = -11;
out:
    return flag;
}

/* The palette manager's data, defined here after the code (a definition after the last
 * function moves no symbol position). The six initialised words land at DS:0x4404-0x4414 in
 * this order (the module has no literals; the asm modules' data follows to 0x4438); the rest
 * are communals (BSS). */
Shadow *master_shadow = 0;                     /* DS:0x4404 */
i16 palette_locked = 0;                            /* DS:0x4408 */
i16 palette_ever_initialized = 0;                  /* DS:0x440a */
i32 pal_manager_update = 0;                       /* DS:0x440c: far callback */
i16 pal_manager_active = 0;                        /* DS:0x4410 */
i16 pal_manager_colors = 0;                        /* DS:0x4412 */
i16 palette_high_search_limit;                     /* DS:0x49cc */
i16 palette_low_search_limit;                      /* DS:0x4a88 */
i16 flag_used[32];                                 /* DS:0x5294 */
i16 palette_reserved_top;                          /* DS:0x52d6 */
u8 master_palette[0x300];               /* DS:0x5b20 */
i16 palette_reserved_bottom;                       /* DS:0x5e20 */
i32 color_status[256];                            /* DS:0x6572 */

/* pal_get_hash -- 1074:0288: the squared distance of two colours, each difference a signed
   byte (it wraps), squared signed, summed in 16 bits */
i16 pal_get_hash(u8 *b, u8 *a)
{
    i16 sum = 0, i;
    for (i = 0; i < 3; i++) {
        i8 d = (i8)(u8)(a[i] - b[i]);
        sum = (i16)(sum + (i16)d * d);
    }
    return sum;
}

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

/* MODULE pal_8: MAPEDIT.EXE segment 0ec0 (pal_8.c), file 0x1020a-0x10258, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pal_load_2   0x1020a
 *
 * No VICEROY twin: reconstructed from the shipped bytes. "elg" (fh in di; plain flags put it in si).
 *
 * pal_load_2(name, pal) (_fastcall) reads a 768-byte palette file into the far buffer
 * through env_open; it returns 1 when the file opened and the read succeeded
 * (fileio_fread_f returning 0), else 0.
 */

/* pal_load_2 -- 0ec0:000a, 78 bytes */
i16 pal_load_2(char *name, char *pal)
{
    i16 ok;
    MSC_FILE *fh;

    ok = 1;
    fh = env_open(name, "rb");
    if (fh != 0) {
        if (fileio_fread_f(pal, 0x300L, 1L, fh) != 0)
            ok = 0;
    }
    if (fh != 0)
        msc_fclose(fh);
    return ok;
}

/* MODULE mcga_7: MAPEDIT.EXE segment 0d70 (mcga_7.c), file 0xed0a-0xed5e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_setpal  0x0ed0a
 *
 * No VICEROY twin: reconstructed from the shipped bytes (it is set_palette_range with
 * first = 0 and count = 256, plus two I/O-delay jumps per byte).
 *
 * WRITING THE WHOLE PALETTE. mcga_setpal(pal) (_fastcall) writes all 256 DAC entries from
 * the far palette, at most mcga_retrace_max_bytes per vertical retrace with interrupts
 * off; mcga_palette_update is raised while it runs. This module also defines the mcga
 * retrace globals (DS:0x3aae-0x3ab8).
 */

i16 mcga_retrace_computed = 0;                   /* DS:0x3aae (defined here, the six in address order) */
u16 mcga_retrace_ticks = 0;                 /* DS:0x3ab0 */
i16 mcga_retrace_max_colors = 256;               /* DS:0x3ab2 */
i16 mcga_retrace_max_bytes = 768;                /* DS:0x3ab4 */
i16 mcga_palette_update = 0;                     /* DS:0x3ab6 */
i16 mcga_palette_fast = 1;                       /* DS:0x3ab8 */
/* mcga_setpal -- 0d70:000a, 84 bytes */
/* PORT: mcga_setpal is in port/ (inline assembly) */

/* MODULE mcga_8: MAPEDIT.EXE segment 105c (mcga_8.c), file 0x11bca-0x11c1a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_getpal  0x11bca
 *
 * Near twin: `ovl30_78548` read_palette (pascal there; _fastcall here).
 *
 * READING THE WHOLE PALETTE. mcga_getpal(pal) (_fastcall) reads the 256 DAC entries into
 * the far buffer, 64 bytes per vertical retrace with interrupts off and two I/O-delay
 * jumps per byte; mcga_palette_update is raised while it runs.
 */

/* mcga_getpal -- 105c:000a, 80 bytes */
/* PORT: mcga_getpal is in port/ (inline assembly) */

/* MODULE mcga_b: MAPEDIT.EXE segment 1061 (mcga_b.c), file 0x11c1a-0x11c96, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_retrace      0x11c1a
 * fn mcga_setpal_range 0x11c2e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c2e).
 *
 * THE VGA PALETTE (madsdev mcga_b.c).
 *   - mcga_retrace waits for the start of a vertical retrace.
 *   - mcga_setpal_range(first, count, pal) (_fastcall) writes `count` DAC entries from
 *     `first`, at most mcga_retrace_max_bytes per retrace with interrupts off.
 *     mcga_palette_update is -1 while it runs (VICEROY's set_palette_range raises 1).
 */

/* mcga_retrace -- 1061:000a, 20 bytes */
/* PORT: mcga_retrace is in port/ (inline assembly) */

/* mcga_setpal_range -- 1061:001e, 104 bytes */
/* PORT: mcga_setpal_range is in port/ (inline assembly) */

/* MODULE mcga_c: MAPEDIT.EXE segment 0d75 (mcga_c.c), file 0xed5e-0xeef0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn mcga_time_palette_swap          0x0ed5e
 * fn mcga_compute_retrace_parameters 0x0edc4
 *
 * Near twins: `ovl30_77ed0` dac_write_range / dac_calibrate (VICEROY's later
 * copy of this module).
 *
 * TIMING THE PALETTE WRITE (madsdev mcga_c.c).
 *   - mcga_time_palette_swap(first, count, pal) (_fastcall) writes `count` DAC entries from
 *     `first` with interrupts off, timed against PIT channel 0, and returns the elapsed
 *     ticks. With mcga_palette_fast set it uses one `rep outsb`; otherwise a plain
 *     outsb/loop (VICEROY adds two I/O-delay jumps per byte).
 *   - mcga_compute_retrace_parameters measures how many palette entries fit in one frame:
 *       1. it times 32 vertical retraces against PIT channel 0 (inline asm) and keeps the
 *          average as mcga_retrace_ticks;
 *       2. it reads the palette and binary-searches the largest count the timed write
 *          fits within a frame (the step starts at 0x40 and halves, 0x20 tries at most;
 *          VICEROY allows 0x40);
 *       3. it records count * 14 (0x20 when nothing fitted) as mcga_retrace_max_colors,
 *          three times that as mcga_retrace_max_bytes, and sets mcga_retrace_computed.
 *     Its frame is 0x60a bytes: a second, unused 768-byte buffer (VICEROY's copy has only
 *     the one, frame 0x30a).
 */

/* mcga_time_palette_swap -- 0d75:000e, 102 bytes */
/* PORT: mcga_time_palette_swap is in port/ (inline assembly) */

/* mcga_compute_retrace_parameters -- 0d75:0074, 300 bytes */
/* 16 pads: the `spent > mcga_retrace_ticks` compare loads the global into ax (position mod 17);
 * at 0..13 pads ours is `cmp [ticks],ax`. */
/* PORT: mcga_compute_retrace_parameters is in port/ (inline assembly) */

/* MODULE mcga_d: MAPEDIT.EXE segment 12ad (mcga_d.c), file 0x140dc-0x14100, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn MCGA_SHAKE   0x140dc
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * SCREEN SHAKE. MCGA_SHAKE() steps a linear-congruential seed (x * 5 + 1), keeps its top
 * two bits of the high byte as a 0..0x3ff display-start offset, and writes it to CRTC
 * register 0x0d (start address low, with the high byte in ah going to 0x0e through the
 * word out). When MCGA_SHAKES counts down to zero the high byte is cleared, so the
 * last shake lands within the first 256 bytes. Entirely inline assembly.
 */

static i16 shake_seed = 1234;                    /* DS:0x4510: module static (no public) */
/* MCGA_SHAKE -- 12ad:000c, 36 bytes */
/* PORT: MCGA_SHAKE is in port/ (inline assembly) */

/* MODULE magic_e: MAPEDIT.EXE segment 133e (magic_e.c), file 0x149ea-0x14a34, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn magic_hash_color 0x149ea
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * magic_hash_color(rgb) returns r*38 + g*76 + b*14 for a far RGB triple: a luminance-like
 * hash (the weights sum to 128). Hand-written: shifts and adds in cx/si, `xor` for the
 * zeroes (MSC's own zero is `sub`).
 */
/* magic_hash_color -- 133e:000a, 74 bytes */
/* PORT: magic_hash_color is in port/ (inline assembly) */

/* magic_hash_color -- 133e:000a: 38 r + 76 g + 14 b, in 16 bits */
i16 magic_hash_color(const void *color)
{
    const u8 *rgb = color;
    return (i16)(u16)(38u * rgb[0] + 76u * rgb[1] + 14u * rgb[2]);
}

/* MODULE cycle_1: MAPEDIT.EXE segment 0f1a (cycle_1.c), file 0x107aa-0x1096a, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn cycle_init   0x107aa
 * fn cycle_colors 0x10846
 *
 * Near twin: `root_0a0a` cycle_start / cycle_tick (VICEROY's copy keeps the
 * palette far and reads the ranges from a loaded file; here the ranges are copied in and
 * the cycling palette is a copy of master_palette).
 *
 * PALETTE COLOUR CYCLING (madsdev cycle_1.c). cycle_list holds up to eight ranges
 * {count, pos, start, speed}.
 *   - cycle_init(ranges, on) copies the 34-byte range list and the 768-byte master palette
 *     in, stamps every range with the current tick, totals the cycled colours
 *     (total_cycle_colors), sets cycling_threshold (3, or 0 for 16 colours or fewer) and
 *     turns cycling on or off.
 *   - cycle_colors, called per tick, rotates each due range of the cycling palette one
 *     entry (inline assembly, through a 3-byte temp) and uploads the cycled part with
 *     mcga_setpal_range once cycling_delay reaches the threshold.
 */

ColorCycle cycle_list;                   /* DS:0x5afe (BSS) */
i32 cycle_timing[8];                    /* DS:0x52e8 (BSS) */
i16 total_cycle_colors;                  /* DS:0x4e9a (BSS) */
i16 cycling_threshold;                   /* DS:0x4b10 (BSS) */
i16 cycling_active = 0;                  /* DS:0x3c64 (defined here) */
u8 cycling_palette[0x300];    /* DS:0x6048 (BSS) */
/* cycle_init -- 0f1a:000a, 156 bytes */
void cycle_init(ColorCycle *ranges, i16 on)
{
    i32 now;
    i16 i;

    now = timer_read();
    for (i = 0; i < 8; i++)
        cycle_timing[i] = now;
    msc__fmemcpy(&cycle_list, ranges, 0x22);
    msc__fmemcpy(cycling_palette, master_palette, 0x300);
    total_cycle_colors = 0;
    for (i = 0; i < cycle_list.n; i++) {
        total_cycle_colors += cycle_list.r[i].count;
        cycle_list.r[i].pos = 0;
    }
    cycling_threshold = 3;
    if (total_cycle_colors <= 16)
        cycling_threshold = 0;
    cycling_active = on;
}
/* cycle_colors -- 0f1a:00a6, 292 bytes */
i16 cycling_delay = 0;                   /* DS:0x3c66 (defined here) */
static u8 cycle_rgb[3];       /* DS:0x49a6: module static (no public) */

/* PORT: cycle_colors is in port/ (inline assembly) */

/* cycle_colors -- 0f1a:00a6: every due range of the cycling palette turns by one colour (its
   last moves to its front), and the cycled colours go to the DAC */
void cycle_colors(void)
{
    i32 now;
    i16 i, changed = 0;
    if (cycling_active == 0)
        return;
    if (++cycling_delay < cycling_threshold)
        return;
    if (mcga_palette_update != 0)
        return;
    now = timer_read();
    for (i = 0; i < cycle_list.n; i++) {
        CycleRange *r = &cycle_list.r[i];
        if (r->speed + cycle_timing[i] <= now) {
            i16 cnt = r->count, start = r->start;
            u16 pos = r->pos;
            cycle_timing[i] = now;
            changed = -1;
            if (cnt > 1) {
                u8 *p = cycling_palette + start * 3, last[3];
                memcpy(last, p + (cnt - 1) * 3, 3);
                memmove(p + 3, p, (size_t)(cnt - 1) * 3);
                memcpy(p, last, 3);
                pos++;
                if (pos >= (u16)cnt)
                    pos = 0;
            }
            r->pos = (u8)pos;
        }
    }
    if (changed)
        mcga_setpal_range(cycle_list.r[0].start, total_cycle_colors, (char *)cycling_palette);
    cycling_delay = 0;
}

/* mcga.c -- the palette routines of mcga_7/8/b/c/d.c whose port access was inline assembly.
 *
 * The VGA's DAC is the host's (host_set_dac). The originals write it a chunk at a time, each
 * chunk after the start of a vertical retrace, so that colour changes never tear: a palette
 * of 768 bytes takes as many frames as it has chunks. The port waits for the next 70 Hz frame
 * the same way, so fades last as long as they did. How much fits in one retrace was measured
 * on the machine (mcga_compute_retrace_parameters); the port reports a fast one, as the
 * measurement would on anything later than a 386: every colour in one retrace.
 */

/* mcga_retrace -- 1061:000a: wait for the next vertical retrace to begin */
void mcga_retrace(void)
{
    u32 frame = host_ms() * 70u / 1000u;
    while (host_ms() * 70u / 1000u == frame) {
        host_pump();
        timer_sync();
        host_sleep(1);
    }
}

/* the chunked DAC write of mcga_setpal and mcga_setpal_range */
static void dac_write(i16 first, const u8 *rgb, u16 bytes, u16 chunk)
{
    u16 colour = (u16)first * 3;
    if (chunk == 0)
        chunk = 0x300;
    while (bytes) {
        u16 n = bytes > chunk ? chunk : bytes, i;
        mcga_retrace();
        for (i = 0; i < n; i++, colour++) {
            u8 c[3];
            host_get_dac(colour / 3, 1, c);
            c[colour % 3] = rgb[i];
            host_set_dac(colour / 3, 1, c);
        }
        rgb += n;
        bytes -= n;
    }
    host_vram_touched();
}

/* mcga_setpal -- 0d70:000a: all 256 colours, 768 bytes of 6-bit components */
void mcga_setpal(char *pal)
{
    mcga_palette_update = 1;
    dac_write(0, (const u8 *)pal, 0x300, (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* mcga_setpal_range -- 1061:001e: count colours from first, read at pal + first * 3 */
void mcga_setpal_range(i16 first, i16 count, char *pal)
{
    mcga_palette_update = -1;
    dac_write(first, (const u8 *)pal + first * 3, (u16)(count * 3), (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* mcga_getpal -- 105c:000a: the DAC's 768 bytes, read 64 at a time, one chunk per retrace */
void mcga_getpal(char *pal)
{
    u16 i;
    mcga_palette_update = 1;
    for (i = 0; i < 0x300 / 0x40; i++)
        mcga_retrace();
    host_get_dac(0, 256, (u8 *)pal);
    mcga_palette_update = 0;
}

/* mcga_time_palette_swap -- 0d75:000e: write a range of colours and return how many PIT
   ticks it took. The port's DAC takes none. */
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal)
{
    host_set_dac(first, count, (const u8 *)pal + first * 3);
    host_vram_touched();
    return 0;
}

/* mcga_compute_retrace_parameters -- 0d75:0074: how much of the palette fits in a retrace.
   On a fast machine the original's search ends at all 256 colours: count * 14 colours. */
void mcga_compute_retrace_parameters(void)
{
    mcga_palette_fast = 0;
    mcga_retrace_max_colors = 0x100 * 14;
    mcga_retrace_max_bytes = (i16)(mcga_retrace_max_colors * 3);
    mcga_retrace_computed = 1;
}

