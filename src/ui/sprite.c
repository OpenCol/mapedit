/* sprite.c -- sprite series: loading them (.SS files) and drawing their frames. */
#include <stddef.h>

#include "msc.h"
#include "error.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mem.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE sprite_e: MAPEDIT.EXE segment 0ddb (sprite_e.c), file 0xf3b8-0xfd1c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn sprite_series_load 0x0f3b8
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * LOADING A SPRITE SERIES (.SS file). sprite_series_load(name, flags) (_fastcall) opens
 * NAME.SS through the loader (the memory tag is "S$" + six letters of the name, after a
 * leading '*' and an "RM" prefix are skipped), reads the 0x98-byte header and the frame
 * table (16 bytes a frame: offset, size, x0, y0, w, h), sizes the SpriteSeries record
 * (0x42 + 12 a frame, +0x68 for the header's extra block, + (count + 0x22) * 8 for a packed
 * file, + the pixel data unless flag 2 or packed) and takes it from sprite_force_memory when
 * that fits sprite_force_size, else from mem_get_name. The palette: a "$color$" list read
 * from the file (or into put_palette when the header says it carries none), then either
 * remapped by name (flag 8: each colour matched against the first four master_palette
 * entries, the rest given the spare colours from the based table) or allocated through
 * pal_allocate, and the frames are recoloured (sprite_color_translate). A packed file keeps
 * its loader open and records the file position of every frame (SpriteInfo at +0x2e,
 * FrameSize table at +0x32, flags table at +0x36) and takes an "$arena$" the size of the
 * largest frame. sprite_error is -1 (open), -2 (read), -4 (memory), -9 (palette).
 *
 *  Written
 *     as a named local it is stored before the pushes; the shipped `mov cx,ax; mov bx,dx ...
 *     mov di,cx; mov [bp-0x228],di` is the CSE temp spilled around the call.
 *   - The spare-colour table is `unsigned char spare_colors[]`: MSC
 *     keeps its segment in a CONST word (DS:0x48b6, past the runtime's _DATA, which is where
 *     the CONST class lands) and reads `mov es,[word]; mov al,[es:di+OFFSET]` with the
 *     array at offset 0 of its own 16-byte segment (15e6:0000, right below DGROUP). A
 *     `u16` variable gives a _DATA word instead; a far array a SEG immediate.
 *   - Two block-scope pad groups: 3 before / 14 after `series` (the `series + size_b`
 *     chains load size_b first) and one before `totalk` (`total <= sprite_force_size` loads
 *     the global). Nothing else moved either site.
 *   - `info->kind == 1 || info->kind == 2` on the byte just stored gives the shipped
 *     `mov cx,ax; dec al; jz; cmp cl,2` (a switch or a test of fh.kind gives cbw/dec/dec).
 *   - The packed-file pointers are a chain: `info = series->info = base + 0xfc; sizes =
 *     series->sizes = info + 0x14` (member stored before the local, then `add ax,0x14`).
 *   - The zero-arena arm is parked out of line: `if (!(flags & 2)) {...} else arena = 0;`.
 *   - `_fmemcpy(master_palette + spare * 3, COLOR + 2, 3)`: a colour not among the first
 *     four master entries is written INTO the master palette at its spare slot.
 */

typedef struct { i32 offset; i32 size; i16 x0; i16 y0; i16 w; i16 h; } FrameEntry;
typedef struct {
    char packed;                /* +0 */
    char b1;                    /* +1 */
    i16 x2;                     /* +2 */
    i16 x4;                     /* +4 */
    i16 tab[16];                /* +6: tab[2] marks the extra block, tab[3] no palette */
    i16 count;                  /* +0x26 */
    char extra[0x68];           /* +0x28 */
    i16 x90;
    i16 x92;
    i32 data_size;             /* +0x94 */
    char unused[16];            /* the local is 0xa8 bytes; 0x98 are read */
} SpriteHeader;
/* PORT: where a packed series' SpriteInfo goes: after the 0xfc colour flags, aligned for its
   pointer */
#define PACKED_INFO(size_b) ((((u32)(size_b) + 0xfc) + 7u) & ~7u)
/* colour i of a list as bytes: +2 its r, g, b; +5 its index (a CSE temp in the shipped code) */
#define COLOR(pal, i) ((char *)(pal) + (i) * 6)

i16 sprite_error = 0;                              /* DS:0x3ade */
char *put_palette = 0;                         /* DS:0x3ae0 */
char *sprite_force_memory = 0;                 /* DS:0x3ae4 */
i32 sprite_last_quantity;                         /* DS:0x5a9a */
i32 sprite_force_size;                            /* DS:0x5ec2 */

/* the spare colours a named palette gets when a colour is not one of the first four */
u8 spare_colors[] = {7, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb};
/* sprite_series_load -- 0ddb:0008, 2404 bytes */
SpriteSeries *sprite_series_load(char *name, i16 flags)
{
    register i16 i;
    i16 k5;
    char name_buf5[80];
    i16 tabsize;
    char *palp8;
    fpos_t pos3;
    FrameSize *sizes7;
    i16 idx;
    FrameEntry *table3;
    SpriteSeries *series;
    i32 size1;
    SpriteHeader hdrn;
    char *cur2;
    i32 size_b;
    SpriteInfo *info;
    i32 runningl;
    PalList *pal;
    i16 found2;
    i32 entsizel;
    i32 maxsizem;
    char *bitsl;
    char tag_buf6[20];
    char *pn;
    FileIO fh;
    i16 hdrsize6;
    i16 j5;
    i32 totalk;
    SpriteSeries *result;

    table3 = 0;
    series = 0;
    result = 0;
    pal = 0;
    mem_last_alloc_loader = 0xd;
    fh.msc_open = 0;
    msc_strcpy(name_buf5, name);
    if (msc_strchr(name_buf5, '.') == 0)
        msc_strcat(name_buf5, ".SS");
    msc_strcpy(tag_buf6, "S$");
    pn = name_buf5;
    msc_strupr(name_buf5);
    if (name_buf5[0] == '*')
        pn = name_buf5 + 1;
    if (pn[0] == 'R' && pn[1] == 'M')
        pn += 2;
    msc_strncat(tag_buf6, pn, 6);
    if (loader_open("rb", 1, &fh, name_buf5) != 0) {
        sprite_error = -1;
        goto done;
    }
    sprite_error = -2;
    hdrsize6 = 0x98;
    if (loader_read((i32)hdrsize6, &hdrn, 1L, &fh) == 0)
        goto done;
    if (hdrn.tab[2] != 0)
        flags |= 4;
    tabsize = hdrn.count << 4;
    /* PORT: the series' size from the port's records; 1994: 0x42 + 12 a frame, and when
       packed (count + 0x22) * 8 = 0xfc colour flags + a 0x14-byte SpriteInfo + 8 a frame */
    size1 = (i32)(offsetof(SpriteSeries, icon) + (u16)hdrn.count * sizeof(IconRec));
    if (flags & 4)
        size1 += 0x68;
    size_b = size1;
    if (hdrn.packed != 0)
        size1 += (i32)(PACKED_INFO(size_b) - size_b + sizeof(SpriteInfo) + (u16)hdrn.count * sizeof(FrameSize));   /* PORT */
    totalk = size1;
    if (!(flags & 2) && hdrn.packed == 0)
        totalk += hdrn.data_size;
    if (sprite_force_memory != 0 && totalk <= sprite_force_size)
        series = (SpriteSeries *)sprite_force_memory;
    sprite_last_quantity = totalk;
    if (series == 0)
        series = mem_get_name(totalk, tag_buf6);
    if (series == 0) {
err4:
        sprite_error = -4;
        goto done;
    }
    table3 = mem_get_name((i32)tabsize, "$sp-load");
    if (table3 == 0)
        goto err4;
    series->extra = 0;
    series->color_flags = 0;
    series->info = 0;
    series->sizes = 0;
    series->arena = 0;
    if (loader_read((i32)tabsize, table3, 1L, &fh) == 0) {
        sprite_error = -2;
        goto done;
    }
    if (hdrn.tab[3] == 0) {
        entsizel = fh.ent[fh.next].size;
        pal = mem_get_name(entsizel, "$color$");
        if (pal == 0)
            goto err4;
        if (loader_read(entsizel, pal, 1L, &fh) == 0)
            goto done;
    } else {
        entsizel = fh.ent[fh.next].size;
        if (put_palette != 0) {
            palp8 = put_palette;
            pal = 0;
            if (loader_read(0x300L, palp8, 1L, &fh) == 0)
                goto done;
        } else {
            msc_fgetpos(fh.fp, &pos3);
            idx = fh.next++;
            msc_fseek(fh.fp, fh.ent[idx].packed + pos3, 0);
        }
    }
    series->packed = hdrn.packed;
    if (hdrn.x2 != 0 && hdrn.x4 < 4)
        series->flag0 = 1;
    else
        series->flag0 = 0;
    series->x2 = hdrn.x4;
    series->count = hdrn.count;
    series->x28 = hdrn.x90;
    series->x2a = hdrn.x92;
    for (i = 0; i < 16; i++)
        series->tab[i] = hdrn.tab[i];
    if (flags & 4) {
        series->extra = (char *)series + size_b - 0x68;
        series->extra = mem_normalize(series->extra);
        msc__fmemcpy(series->extra, hdrn.extra, 0x68);
    }
    bitsl = mem_normalize((char *)series + size1);
    cur2 = bitsl;
    for (i = 0; i < series->count; i++) {
        series->icon[i].x0 = table3[i].x0;
        series->icon[i].y0 = table3[i].y0;
        series->icon[i].w = table3[i].w;
        series->icon[i].h = table3[i].h;
        if (!(flags & 2) && hdrn.packed == 0) {
            series->icon[i].data = cur2;
            cur2 = mem_normalize(cur2 + table3[i].size);
        } else
            series->icon[i].data = 0;
    }
    if (!(flags & 2) && hdrn.packed == 0) {
        if (loader_read(hdrn.data_size, bitsl, 1L, &fh) == 0)
            goto done;
    }
    if (hdrn.tab[3] == 0) {
        if (flags & 9) {
            series->pal_handle = 0;
            if (flags & 8) {
                j5 = 0;
                for (i = 0; i < pal->n; i++) {
                    found2 = 0;
                    k5 = 0;
                    while (found2 == 0) {
                        if (k5 >= 4)
                            goto no_match;
                        if (msc__fmemcmp(COLOR(pal, i) + 2, master_palette + k5 * 3, 3) == 0) {
                            found2 = 1;
                            COLOR(pal, i)[5] = (char)k5;
                        }
                        k5++;
                    }
no_match:
                    if (found2 == 0) {
                        msc__fmemcpy(master_palette + spare_colors[j5] * 3, COLOR(pal, i) + 2, 3);
                        COLOR(pal, i)[5] = spare_colors[j5];
                        j5 = j5 + 1 <= 6 ? j5 + 1 : 6;
                    }
                }
                sprite_color_translate(series, (char *)pal);
            }
        } else {
            series->pal_handle = pal_allocate(flags & 0xfc00, pal, master_shadow);
            if (series->pal_handle < 0) {
                sprite_error = -9;
                goto done;
            }
            if (!(flags & 2) && hdrn.packed == 0)
                sprite_color_translate(series, (char *)pal);
        }
    }
    if (hdrn.packed != 0) {
        series->color_flags = (char *)series + size_b;
        info = series->info = (SpriteInfo *)((char *)series + PACKED_INFO(size_b));   /* PORT */
        sizes7 = series->sizes = (FrameSize *)(info + 1);   /* PORT: 0x14 in 1994 */
        for (i = 0; i < pal->n; i++)
            series->color_flags[i] = pal->e[i].index;
        info->b1 = hdrn.b1;
        info->kind = fh.kind;
        info->fp = fh.fp;
        msc_fgetpos(fh.fp, &runningl);
        info->pos = runningl;
        maxsizem = 0;
        for (i = 0; i < series->count; i++) {
            sizes7[i].offset = table3[i].offset;
            sizes7[i].size = table3[i].size;
            maxsizem = sizes7[i].size > maxsizem ? sizes7[i].size : maxsizem;
        }
        if (!(flags & 2)) {
            if (pal != 0)
                mem_free(pal);
            pal = 0;
            if (table3 != 0)
                mem_free(table3);
            table3 = 0;
            series->arena = mem_get_name(maxsizem, "$arena$");
            if (series->arena == 0)
                goto done;
            msc__fmemcpy(&series->tab[0], &maxsizem, 4);
            fh.msc_open = 0;
        } else
            series->arena = 0;
    }
    result = series;
done:
    if (fh.msc_open != 0)
        loader_close(&fh);
    if (pal != 0)
        mem_free(pal);
    if (table3 != 0)
        mem_free(table3);
    if (series != 0 && sprite_force_memory != (char *)series && result == 0)
        mem_free(series);
    return result;
}

/* sprite_color_translate -- 1069:0006: rewrite every frame's colours in place through map: a
   colour c becomes map[5 + 6c] (the palette list's index field). A frame's rows end with a
   row opening FCh; FFh is an empty row, FDh a row of runs, anything else a plain row whose
   FEh runs keep their count. Transparent FDh pixels stay. A frame with no pixels ends it. */
void sprite_color_translate(SpriteSeries *sheet, char *map)
{
    SpriteSeries *ss = sheet;
    const u8 *tab = (const u8 *)map + 5;
    i16 i;
    for (i = 0; i < ss->count; i++) {
        u8 *s = ss->icon[i].data;
        if (s == 0)
            break;
        for (;;) {
            u8 mode = *s++;
            if (mode == 0xfc)
                break;
            if (mode == 0xff)
                continue;
            if (mode == 0xfd) {
                for (;;) {
                    if (*s++ == 0xff)
                        break;
                    if (*s != 0xfd)
                        *s = tab[*s * 6];
                    s++;
                }
            } else {
                for (;;) {
                    if (*s == 0xff) {
                        s++;
                        break;
                    }
                    if (*s == 0xfe)
                        s += 2;
                    if (*s != 0xfd)
                        *s = tab[*s * 6];
                    s++;
                }
            }
        }
    }
}

/* sprite.c -- the sprite drawers of sprite_1.c (and their kin), MicroProse's run-length
 * sprites, whose bodies were one block of inline assembly each.
 *
 * A sprite series (sprite_e.c's SpriteSeries) has a record per frame: its pixels (a pointer),
 * and its width and height. Callers number frames from 1: the original reaches frame n's
 * record at sheet + 36h + 12n, the series' icon[n - 1]. Bit 15 set (n | 8000h) draws the
 * frame mirrored left to right.
 *
 * A frame's pixels are rows of bytes, each row opening with a mode byte:
 *   FFh  an empty row (nothing follows);
 *   FDh  runs: (count, colour) pairs, until FFh ends the row;
 *   else plain: one colour a byte, FEh count colour a run, FFh ends the row.
 * Colour FDh is transparent. Only the columns inside the clip are drawn, and only they move
 * the destination; a row cut short by the clip skips its remaining bytes up to its FFh. All
 * of that, the clip arithmetic of a mirrored frame included, is the original's, step for
 * step.
 */


static IconRec *frame(SpriteSeries *sheet, i16 n)
{
    return &sheet->icon[n - 1];
}

/* the shared decoder; with under set (sprite_draw_black) a pixel is written only where the
   destination is still 0, under what is drawn already -- the destination moves either way */
static void draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, int under)
{
    i16 dir = 1, maxx1 = (i16)(port->w - 1), maxy = (i16)(port->h - 1);
    i16 iwo, ihn, skipx, visw, endx, skipyo, vish, endyo, bx, cx, ax, dx;
    IconRec *rec;
    const u8 *s;
    char *row;
    u16 r;

    if (icon < 0)
        dir = -1;
    icon = (i16)(icon & 0x7fff);
    rec = frame(sheet, icon);
    s = rec->data;
    iwo = rec->w;
    ihn = rec->h;

    /* columns */
    bx = iwo;
    cx = 0;
    ax = x;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxx1);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipx = cx;
    visw = bx;
    cx = (i16)(cx + bx);
    endx = cx;
    if (bx <= 0)
        return;
    if (dir != 1) {
        i16 di = iwo;
        ax = (i16)(ax + di - 1);
        x = ax;
        di = (i16)(di - cx);
        di = (i16)-di;
        skipx = di;
        di = (i16)-di;
        di = (i16)(di + visw);
        endx = di;
    }

    /* rows */
    bx = ihn;
    cx = 0;
    ax = y;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxy);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipyo = cx;
    vish = bx;
    endyo = (i16)(cx + bx);
    if (bx <= 0)
        return;

    row = port->bits + (u32)(u16)(cx + ax) * (u16)port->w;
    row += x + skipx;
    {
        i16 first = (i16)(skipx * dir), last = endx;
        i16 rowno;
        for (rowno = 0; rowno < endyo; rowno++) {
            int eol = 0;
            if (rowno >= skipyo) {
                char *d = row;
                i16 col = 0;
                u8 mode = *s++;
                if (mode == 0xff)
                    eol = 1;
                else if (mode == 0xfd) {
                    while (col < last) {
                        u8 n = *s++, c;
                        if (n == 0xff) {
                            eol = 1;
                            break;
                        }
                        c = *s++;
                        do {
                            if (col >= first && col < last) {
                                if (c != 0xfd && !(under && *d))
                                    *d = (char)c;
                                d += dir;
                            }
                            col++;
                        } while (--n);
                    }
                } else {
                    while (col < last) {
                        u8 c = *s++;
                        if (c == 0xff) {
                            eol = 1;
                            break;
                        }
                        if (c == 0xfe) {
                            u8 n = *s++;
                            c = *s++;
                            do {
                                if (col >= first && col < last) {
                                    if (c != 0xfd && !(under && *d))
                                        *d = (char)c;
                                    d += dir;
                                }
                                col++;
                            } while (--n);
                            continue;
                        }
                        if (col >= first && col < last) {
                            if (c != 0xfd && !(under && *d))
                                *d = (char)c;
                            d += dir;
                        }
                        col++;
                    }
                }
                row += port->w;
            }
            if (!eol)
                while (*s++ != 0xff)
                    ;
        }
    }
    (void)vish;
    (void)r;
}

/* sprite_draw -- 0d8f:0000 */
void sprite_draw(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y)
{
    draw(icon, sheet, port, x, y, 0);
}

/* sprite_draw_black -- 0e71:000c: the same, only onto pixels that are 0 */
void sprite_draw_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y)
{
    draw(icon, sheet, port, x, y, 1);
}

/* sprite_draw_scaled -- 0dae:000a: the frame shrunk to scale percent. A table over the
   frame's larger dimension keeps a source row or column when an accumulator (from 50, plus
   scale each step) reaches 100; the output is centred on x and its bottom row is y. Dropped
   columns move only the source; kept ones are clipped and drawn as sprite_draw's are. */
static void draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale, int under)
{
    i16 dir = 1, maxx1 = (i16)(port->w - 1), maxy = (i16)(port->h - 1);
    i16 iwo, ihn, maxdim, outw = 0, outh = 0, skipx, visw, endx, skipyo, endyo;
    i16 bx, cx, ax, dx, di, acc;
    u8 keep[320];
    IconRec *rec;
    const u8 *s;
    char *row;

    if (icon < 0)
        dir = -1;
    icon = (i16)(icon & 0x7fff);
    rec = frame(sheet, icon);
    s = rec->data;
    iwo = rec->w;
    ihn = rec->h;
    maxdim = iwo >= ihn ? iwo : ihn;
    acc = 50;
    for (di = 0; di < maxdim; di++) {
        acc = (i16)(acc + scale);
        if (acc >= 100) {
            if (di < (i16)sizeof keep)
                keep[di] = 0xff;
            acc = (i16)(acc - 100);
            if (di < iwo)
                outw++;
            if (di < ihn)
                outh++;
        } else if (di < (i16)sizeof keep)
            keep[di] = 0;
    }
    x = (i16)(x - ((u16)outw >> 1));
    y = (i16)(y - outh + 1);

    bx = outw;
    cx = 0;
    ax = x;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxx1);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipx = cx;
    visw = bx;
    cx = (i16)(cx + bx);
    endx = cx;
    if (bx <= 0)
        return;
    if (dir != 1) {
        i16 d2 = outw;
        ax = (i16)(ax + d2 - 1);
        x = ax;
        d2 = (i16)(d2 - cx);
        d2 = (i16)-d2;
        skipx = d2;
        d2 = (i16)-d2;
        d2 = (i16)(d2 + visw);
        endx = d2;
    }
    bx = outh;
    cx = 0;
    ax = y;
    dx = (i16)(ax + bx - 1);
    if (ax < 0) {
        bx = (i16)(bx + ax);
        cx = (i16)(cx - ax);
    }
    dx = (i16)(dx - maxy);
    if (dx > 0)
        bx = (i16)(bx - dx);
    skipyo = cx;
    endyo = (i16)(cx + bx);
    if (bx <= 0)
        return;

    row = port->bits + (u32)(u16)(cx + ax) * (u16)port->w;
    row += x + skipx;
    skipx = (i16)(skipx * dir);
    {
        i16 srow, orow = -1;
        for (srow = 0; srow < ihn; srow++) {
            int eol = 0;
            if (keep[srow]) {
                if (++orow >= endyo)
                    return;
                if (orow >= skipyo) {
                    char *d = row;
                    i16 col = 0, out = 0;
                    u8 mode = *s++;
#define PIXEL(c) do { \
        if (keep[col]) { \
            if (out >= skipx && out < endx) { \
                if ((c) != 0xfd && !(under && *d)) \
                    *d = (char)(c); \
                d += dir; \
            } \
            out++; \
        } \
        col++; \
    } while (0)
                    if (mode == 0xff)
                        eol = 1;
                    else if (mode == 0xfd) {
                        while (out < endx) {
                            u8 n = *s++, c;
                            if (n == 0xff) {
                                eol = 1;
                                break;
                            }
                            c = *s++;
                            do PIXEL(c); while (--n);
                        }
                    } else {
                        while (out < endx) {
                            u8 c = *s++;
                            if (c == 0xff) {
                                eol = 1;
                                break;
                            }
                            if (c == 0xfe) {
                                u8 n = *s++;
                                c = *s++;
                                do PIXEL(c); while (--n);
                                continue;
                            }
                            PIXEL(c);
                        }
                    }
#undef PIXEL
                    row += port->w;
                }
            }
            if (!eol)
                while (*s++ != 0xff)
                    ;
        }
    }
}

void sprite_draw_scaled(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale)
{
    draw_scaled(icon, sheet, port, x, y, scale, 0);
}

/* sprite_draw_scaled_black -- 0e92:000a: scaled, only onto pixels that are 0 */
void sprite_draw_scaled_black(i16 icon, SpriteSeries *sheet, DosPort *port, i16 x, i16 y, i16 scale)
{
    draw_scaled(icon, sheet, port, x, y, scale, 1);
}
