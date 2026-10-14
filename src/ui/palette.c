/* palette.c -- the palette: allocating colours, shadows, the stock colours, the DAC, cycling.
 *
 * master_palette holds the 256 colours the editor shows, three bytes each (red, green, blue,
 * 0..3Fh, as the VGA's DAC takes them). The file has five parts:
 *
 * The palette manager shares master_palette among sprite series. An allocation is a "flag",
 * a number 0..31, and color_status holds a 32-bit word per entry with a bit per flag using
 * the entry; a free entry's word is 0. Three bits have fixed meanings, so pal_init reserves
 * their flags: bit 0 a stock colour (the reserved ranges at the bottom and top), bit 1 a
 * colour one series needs to itself, bit 31 locked (pal_lock). pal_allocate takes a free
 * flag (flag_used says which are taken), gives each colour of a request list (a PalList) an
 * entry -- an existing entry of the same or the nearest colour, or a free one it fills --
 * marks those entries with the flag's bit and returns the flag; pal_deallocate releases it.
 * A shadow (Shadow) is up to three colours of a list that take fixed places. Nothing in the
 * editor calls pal_init: until it runs, both search limits are 0 and no flag is reserved.
 * Every change is reported to a callback (pal_manager_update), which the editor never
 * installs.
 *
 * The stock palettes: pal_interface (the sixteen interface colours), pal_white (four greys),
 * pal_grey (a grey ramp) and pal_load_2 (a 768-byte palette file such as VICEROY.PAL).
 *
 * The DAC (mcga_*): the original writes the VGA's colour registers a chunk at a time, each
 * chunk after the start of a vertical retrace, so that colour changes never tear; a palette
 * of 768 bytes takes as many frames as it has chunks. How much fits in one retrace is
 * measured at startup (mcga_compute_retrace_parameters). The port's DAC is the host's
 * (host_set_dac): it waits for the next 70 Hz frame the same way, so fades last as long as
 * they did, and reports a fast machine, as the measurement would on anything later than a
 * 386: every colour in one retrace.
 *
 * Colour cycling: cycle_init takes up to eight ranges of entries and a copy of
 * master_palette; cycle_colors, called on the clock, turns each range whose time has come
 * and writes the cycled colours to the DAC.
 */
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
#pragma pack(2)   /* records packed on 2-byte boundaries, as the 1994 compiler did */

typedef union { i32 l; struct { u8 b0, b1, b2, b3; } b; } ColorStatus;   /* unused */
typedef void (*PalFn)(void);

/* Calls the manager callback fn, when there is one, with pal_manager_active set to code
 * for the duration: 1 reset, 2 allocated, 3 allocation failed, 4 released.
 * Port: fn is a 32-bit value, as the original's far pointer was; it is always 0 here. */
void pal_call_manager(i32 fn, i16 code)
{
    if (fn) {
        pal_manager_active = code;
        (*(PalFn *)&fn)();
        pal_manager_active = 0;
    }
}

/* Resets the palette manager: every entry free, the search limits the whole palette, the
 * sixteen interface colours loaded into master_palette (the first time only), `bottom`
 * entries from 0 and `top` entries from 255 down marked stock, every flag released but 0 and
 * 1, nothing locked. Tells the manager (code 1). */
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

/* Locks the palette: takes flag 31 and sets bit 31 on every entry in use, so that
 * pal_deallocate frees none of them. Reports error -5 when flag 31 is already taken by an
 * allocation (and locks anyway). */
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

/* Unlocks the palette: clears bit 31 on every entry. The original then releases flag 32,
 * one past flag_used, so flag 31 stays taken; the word it clears is map_view_size_x. */
void pal_unlock(void)
{
    i16 i;

    if (palette_locked) {
        for (i = 0; i < 256; i++)
            color_status[i] &= 0x7fffffffL;
        /* Port: flag_used[32] is map_view_size_x in the original's memory; cleared by name */
        extern i16 map_view_size_x;
        palette_locked = map_view_size_x = 0;
    }
}

/* Releases allocation `flag` (1..31; others are ignored): clears its bit from every entry,
 * frees the entries left with bit 1 alone, and tells the manager (code 4) if the flag was
 * taken. Always returns 0. */
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

/* Merges the n allocations in `list` into `flag`: every entry used by one of them loses
 * their bits and takes flag's; the listed flags are released and `flag` is taken. */
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

/* Sorts a shadow's list indices by the brightness of their colours (magic_hash_color),
 * darkest first. */
void pal_shadow_sort(Shadow *sh, PalList *list)
{
    i32 keys[3];
    i16 i;

    for (i = 0; i < sh->n; i++)
        keys[i] = magic_hash_color(list->e + sh->idx[i]);
    sort_insertion(sh->n, sh->idx, keys);
}

/* Fills shadow sh with the indices of the first three colours of list flagged 10h. */
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

/* Makes sh the shadow pal_allocate places shadow colours by. */
void pal_activate_shadow(Shadow *sh)
{
    master_shadow = sh;
}

/* Returns the first free flag, or -10 after reporting error -5 when all 32 are taken. */
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

/* Returns the number of free entries; *first is set to the first of them, or -1. */
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

typedef struct { u8 r, g, b; } Rgb;
/* Gives every colour of `list` a palette entry and stores the entry's number in the colour's
 * `index`. Returns the new allocation's flag, -10 when no flag is free, or -11 (after
 * reporting error -6) when a colour finds no entry; the manager is told 2 or 3.
 *
 * The search covers entries palette_low_search_limit up to palette_high_search_limit,
 * further limited to the first 252 entries (all 256 with 0800h) and, without 4000h, to those
 * outside the reserved bottom and top ranges. Colours are placed in this order: those
 * flagged 80h before the others, within each those flagged neither 20h nor 40h first. For
 * each colour, the first rule that applies:
 *   - flag 08h: entry FDh (the transparent colour);
 *   - flag 10h, with a shadow sh given and not exact: the entry sh holds for the colour's
 *     rank among the list's own shadow colours (pal_init_shadow, sorted by brightness);
 *   - flag 10h, exact (8000h) with a shadow given: entry F0h + its place in master_shadow,
 *     which takes the colour;
 *   - not exact: an entry in use (not a stock one without 4000h; not one needed by another
 *     series unless the colour is flagged 80h) of the nearest colour by pal_get_hash when the
 *     colour is flagged 20h, or flagged 40h (or 2000h is given) and 1000h is given or no
 *     entry is free; otherwise one of exactly the same colour;
 *   - a free entry from the lowest, which takes the colour; with 1000h, only for colours
 *     flagged neither 20h nor 40h and only without 2000h.
 * The entries chosen get the flag's bit, and in an exact request bit 1 as well for colours
 * whose `extra` byte is set. */
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

/* The palette manager's state. */
Shadow *master_shadow = 0;                     /* the shadow pal_allocate uses, or 0 */
i16 palette_locked = 0;                            /* -1 between pal_lock and pal_unlock */
i16 palette_ever_initialized = 0;                  /* -1 once the interface colours are loaded */
i32 pal_manager_update = 0;                       /* the manager callback, or 0 (never set) */
i16 pal_manager_active = 0;                        /* the code during a callback, else 0 */
i16 pal_manager_colors = 0;                        /* colours in the last allocation request */
i16 palette_high_search_limit;                     /* pal_allocate searches below this entry */
i16 palette_low_search_limit;                      /* ... and from this one */
i16 flag_used[32];                                 /* -1 for each allocation flag taken */
i16 palette_reserved_top;                          /* stock entries at the top (pal_init) */
u8 master_palette[0x300];               /* the 256 colours shown, r g b each */
i16 palette_reserved_bottom;                       /* stock entries at the bottom (pal_init) */
i32 color_status[256];                            /* each entry's status bits (see the top) */

/* Returns the squared distance of two colours: each component difference taken as a signed
 * byte (it wraps), squared, summed in 16 bits. */
i16 pal_get_hash(u8 *b, u8 *a)
{
    i16 sum = 0, i;
    for (i = 0; i < 3; i++) {
        i8 d = (i8)(u8)(a[i] - b[i]);
        sum = (i16)(sum + (i16)d * d);
    }
    return sum;
}

/* Fills entries 0..15 of palette pal with the sixteen interface colours, the EGA's: entry
 * 4r + 2g + b has those components at 2Ah (entries 0..7, the others 0) or at 3Fh (8..15, the
 * others 15h); entry 6 then gets green 15h, which makes it brown. */
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

/* Fills entries 0..3 of palette pal with four greys: black, 15h, 2Ah and white (3Fh). */
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

/* Fills `count` entries of palette pal from `start` with a grey ramp from level lo up to
 * hi, which the last entry reaches; a Bresenham accumulator decides where the level steps.
 * hi must not be below lo: the accumulator is compared unsigned. */
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

/* Reads a 768-byte palette file (env_open: the game folder) into pal. Returns 0 when it was
 * read, 1 when the file cannot be opened or is short. */
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

/* The DAC's state. */
i16 mcga_retrace_computed = 0;                   /* set once the retrace has been measured */
u16 mcga_retrace_ticks = 0;                 /* a frame in PIT ticks (unused by the port) */
i16 mcga_retrace_max_colors = 256;               /* colours the DAC takes per retrace */
i16 mcga_retrace_max_bytes = 768;                /* bytes written per retrace: 3 a colour */
i16 mcga_palette_update = 0;                     /* nonzero while a DAC transfer runs */
i16 mcga_palette_fast = 1;                       /* the original's fast write (one rep outsb) */

/* Unused: the original's screen shake seed. MCGA_SHAKE is in video.c and does not shake. */
static i16 shake_seed = 1234;

/* Returns the brightness of an RGB triple: 38 r + 76 g + 14 b (the weights sum to 128), in
 * 16 bits. */
i16 magic_hash_color(const void *color)
{
    const u8 *rgb = color;
    return (i16)(u16)(38u * rgb[0] + 76u * rgb[1] + 14u * rgb[2]);
}

/* Colour cycling's state. */
ColorCycle cycle_list;                   /* the ranges: count, pos, start entry, speed */
i32 cycle_timing[8];                    /* when each range last turned (timer_read) */
i16 total_cycle_colors;                  /* the ranges' counts summed */
i16 cycling_threshold;                   /* cycle_colors works once in this many calls */
i16 cycling_active = 0;                  /* nonzero: cycle_colors runs */
u8 cycling_palette[0x300];    /* the palette being cycled */

/* Starts colour cycling (or only sets it up, with `on` 0): copies the ranges and
 * master_palette, stamps every range with the current time, and sets cycling_threshold to
 * 3, or 0 when the ranges hold 16 colours or fewer. */
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

i16 cycling_delay = 0;                   /* calls of cycle_colors since the last turn */
static u8 cycle_rgb[3];       /* unused: the original's rotation buffer */

/* One step of colour cycling, called on the clock. While cycling is on, once in
 * cycling_threshold calls (every call when it is 0) and when no DAC transfer is running,
 * every range whose speed (in timer ticks) has passed turns by one colour, its last moving
 * to its front. If any turned, total_cycle_colors entries from the first range's start go
 * to the DAC: the ranges are taken to be contiguous. */
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

/* Waits for the next vertical retrace to begin: the next 70 Hz frame. */
void mcga_retrace(void)
{
    u32 frame = host_ms() * 70u / 1000u;
    while (host_ms() * 70u / 1000u == frame) {
        host_pump();
        timer_sync();
        host_sleep(1);
    }
}

/* Writes `bytes` bytes of colour components to the DAC from colour `first`, at most `chunk`
 * (768 when 0) after each retrace. A chunk may end inside a colour, as the original's did. */
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

/* Writes all 256 colours of palette pal (768 bytes, 0..3Fh each) to the DAC. */
void mcga_setpal(char *pal)
{
    mcga_palette_update = 1;
    dac_write(0, (const u8 *)pal, 0x300, (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* Writes `count` colours from `first` to the DAC, read from palette pal at the same place. */
void mcga_setpal_range(i16 first, i16 count, char *pal)
{
    mcga_palette_update = -1;
    dac_write(first, (const u8 *)pal + first * 3, (u16)(count * 3), (u16)mcga_retrace_max_bytes);
    mcga_palette_update = 0;
}

/* Reads the DAC's 256 colours into pal. The original reads 64 bytes per retrace; the port
 * waits as many retraces, then reads them at once. */
void mcga_getpal(char *pal)
{
    u16 i;
    mcga_palette_update = 1;
    for (i = 0; i < 0x300 / 0x40; i++)
        mcga_retrace();
    host_get_dac(0, 256, (u8 *)pal);
    mcga_palette_update = 0;
}

/* Writes `count` colours from `first` and returns how many timer (PIT) ticks it took, which
 * the original used to measure the DAC. The port's DAC takes none. */
i16 mcga_time_palette_swap(i16 first, i16 count, char *pal)
{
    host_set_dac(first, count, (const u8 *)pal + first * 3);
    host_vram_touched();
    return 0;
}

/* Decides how much of the palette is written per retrace. The original times retraces and
 * DAC writes and searches the largest count of colours that fits in a frame, recording
 * count * 14 colours. On a fast machine that search ends at all 256, which the port records
 * directly. */
void mcga_compute_retrace_parameters(void)
{
    mcga_palette_fast = 0;
    mcga_retrace_max_colors = 0x100 * 14;
    mcga_retrace_max_bytes = (i16)(mcga_retrace_max_colors * 3);
    mcga_retrace_computed = 1;
}
