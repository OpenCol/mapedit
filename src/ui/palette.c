/* palette.c -- the palette: allocating colours to sprite series, shadows, the DAC.
 *
 * master_palette holds the 256 colours the editor shows, three bytes each (red, green, blue,
 * 0..3Fh, as the VGA's DAC takes them). The file has three parts:
 *
 * The palette manager shares master_palette among sprite series. An allocation is a "flag",
 * a number 0..31, and color_status holds a 32-bit word per entry with a bit per flag using
 * the entry; a free entry's word is 0. Two bits have fixed meanings: bit 0 a stock colour
 * (the reserved ranges at the bottom and top), bit 1 a colour one series needs to itself.
 * pal_allocate takes a free flag (flag_used says which are taken), gives each colour of a
 * request list (a PalList) an entry -- an existing entry of the same or the nearest colour,
 * or a free one it fills -- marks those entries with the flag's bit and returns the flag.
 * A shadow (Shadow) is up to three colours of a list that take fixed places. Nothing in the
 * editor sets the search limits or the reserved ranges: both limits stay 0 and no flag is
 * reserved. Every allocation is reported to a callback (pal_manager_update), which the
 * editor never installs.
 *
 * pal_load_2 reads a 768-byte palette file such as VICEROY.PAL.
 *
 * The DAC (mcga_*): the original writes the VGA's colour registers a chunk at a time, each
 * chunk after the start of a vertical retrace, so that colour changes never tear; a palette
 * of 768 bytes takes as many frames as it has chunks. How much fits in one retrace is
 * measured at startup (mcga_compute_retrace_parameters). The port's DAC is the host's
 * (host_set_dac): it waits for the next 70 Hz frame the same way, so fades last as long as
 * they did, and reports a fast machine, as the measurement would on anything later than a
 * 386: every colour in one retrace.
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
 * for the duration: 2 allocated, 3 allocation failed (pal_allocate's two outcomes).
 * Port: fn is a 32-bit value, as the original's far pointer was; it is always 0 here. */
void pal_call_manager(i32 fn, i16 code)
{
    if (fn) {
        pal_manager_active = code;
        (*(PalFn *)&fn)();
        pal_manager_active = 0;
    }
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
    i16 nfree3;
    i16 lo8;
    i16 bestidx;
    i16 score9;
    u8 kindsl[0x100];
    i16 exact;
    i16 kk;
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
    i16 allocflag8;
    i16 useshadowj;
    i16 maxcol9;
    i16 found1;
    i16 m;
    i32 flagbit4;
    i16 hil;
    char buf[4];
    register i16 c;
    register i16 ei;

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
i32 pal_manager_update = 0;                       /* the manager callback, or 0 (never set) */
i16 pal_manager_active = 0;                        /* the code during a callback, else 0 */
i16 pal_manager_colors = 0;                        /* colours in the last allocation request */
i16 palette_high_search_limit;                     /* pal_allocate searches below this entry */
i16 palette_low_search_limit;                      /* ... and from this one */
i16 flag_used[32];                                 /* -1 for each allocation flag taken */
i16 palette_reserved_top;                          /* stock entries at the top (never set) */
u8 master_palette[0x300];               /* the 256 colours shown, r g b each */
i16 palette_reserved_bottom;                       /* stock entries at the bottom (never set) */
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
i16 mcga_retrace_max_colors = 256;               /* colours the DAC takes per retrace */
i16 mcga_retrace_max_bytes = 768;                /* bytes written per retrace: 3 a colour */
i16 mcga_palette_update = 0;                     /* nonzero while a DAC transfer runs */
i16 mcga_palette_fast = 1;                       /* the original's fast write (one rep outsb) */

/* Returns the brightness of an RGB triple: 38 r + 76 g + 14 b (the weights sum to 128), in
 * 16 bits. */
i16 magic_hash_color(const void *color)
{
    const u8 *rgb = color;
    return (i16)(u16)(38u * rgb[0] + 76u * rgb[1] + 14u * rgb[2]);
}

/* The colour-cycling switch. Popups clear it while they load a portrait or set the palette,
 * then restore it; nothing else uses it. */
i16 cycling_active = 0;


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
