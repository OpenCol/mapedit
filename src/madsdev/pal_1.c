/* Imported from mapedit-decomp src/madsdev/pal_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
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
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))
// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { u8 r, g, b, index, extra, flags; } PalEntry;
typedef struct { i16 n; PalEntry e[1]; } PalList;
typedef struct { i16 n; i16 idx[3]; } Shadow;
typedef union { i32 l; struct { u8 b0, b1, b2, b3; } b; } ColorStatus;
typedef void (*PalFn)(void);

extern void error_report(i16 code, i16 level, i16 module, i32 data1, i32 data2);
extern void pal_interface(char *pal);
extern i16 magic_hash_color(PalEntry *e);
extern void sort_insertion(i16 n, i16 *idx, i32 *keys);

extern Shadow *master_shadow;                  /* DS:0x4404 */
extern i16 palette_locked;                         /* DS:0x4408 */
extern i16 palette_ever_initialized;               /* DS:0x440a */
extern i32 pal_manager_update;                    /* DS:0x440c: far callback */
extern i16 pal_manager_active;                     /* DS:0x4410 */
extern i16 pal_manager_colors;                     /* DS:0x4412 */
extern i16 palette_high_search_limit;              /* DS:0x49cc */
extern i16 palette_low_search_limit;               /* DS:0x4a88 */
extern i16 flag_used[32];                          /* DS:0x5294 */
extern i16 palette_reserved_top;                   /* DS:0x52d6 */
extern u8 master_palette[0x300];        /* DS:0x5b20 */
extern i16 palette_reserved_bottom;                /* DS:0x5e20 */
extern i32 color_status[256];                     /* DS:0x6572 */
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
        palette_locked = flag_used[32] = 0;
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
i16 pal_get_hash(u8 *b, u8 *a);

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
extern void sort_insertion_8(i16 n, u8 *order, u8 *keys);
typedef struct { u8 r, g, b; } Rgb;
/* 4 pads: the limit compares load the global into ax (`mov ax,[g]; cmp ax,[lo8]`); 0..3 pads
 * compare the other way round (position mod 17). */
extern i16 pad_a0;
extern i16 pad_a1;
extern i16 pad_a2;
extern i16 pad_a3;
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

