/* buffer.c -- 2D byte arrays (DosPort): pixels, rectangles, lines, tiling, and saving what a
 * window covers.
 *
 * A DosPort is h rows of w bytes, row-major, at bits: a screen, an off-screen work buffer, or
 * any other picture. buffer_init/buffer_init_name allocate one and buffer_free releases it.
 * The primitives address pixels through buffer_pointer; buffer_rect_fill and the line drawers
 * clip to the port (buffer_conform clips a rectangle), while the rectangle copies, buffer_put
 * and buffer_get trust their coordinates. Ports are often passed by value: the record is
 * copied, the pixels are shared.
 *
 * buffer_preserve/buffer_restore save the rectangle a menu or popup is about to cover and put
 * it back afterwards. The saved pixels go to a scratch port in memory or, failing that, to one
 * of ten files named $MPOP.$0 to $MPOP.$9 in the current directory, each the rectangle's rows
 * written one after another with no header.
 *
 * In the original the pixel loops were assembly walking a far block 32 KB at a time; here they
 * are plain loops over a flat pointer that write the same pixels in the same order.
 * tests/oracle runs the original beside each of them.
 */
#include "msc.h"
#include "buffer.h"
#include "error.h"
#include "fileio.h"
#include "font.h"
#include "loader.h"
#include "map.h"
#include "mapedit.h"
#include "mem.h"
#include "menu.h"
#include "pack.h"
#include "pal.h"
#include "popup.h"
#include "port_records.h"
#include "records.h"
#include "sprite.h"
#include "units.h"
#include "w16types.h"
#pragma pack(2)   /* records packed on 2-byte boundaries, as in 1994; file formats rely on it */

/* buffer_fill -- fill the whole port with colour c */
void buffer_fill(char c, DosPort port)
{
    buffer_rect_fill(0, 0, port.w, port, port.h, c);
}

/* buffer_put -- set pixel (x, y) of the port to v. No clipping. */
void buffer_put(i16 x, i16 y, i16 v, DosPort port)
{
    *buffer_pointer(&port, x, y) = (char)v;
}

/* buffer_get -- pixel (x, y) of the port. No clipping. */
u8 buffer_get(i16 x, i16 y, DosPort port)
{
    return *buffer_pointer(&port, x, y);
}

/* buffer_init -- allocate w * h bytes for port and record its size. Returns -1, or 0 when
 * the allocation fails (port->bits is then 0 and w and h are left as they were). The pixels
 * start as 0.
 */
i16 buffer_init(DosPort *port, u16 w, u16 h)
{
    port->bits = mem_get((u32)h * w);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}
/* buffer_init_name -- buffer_init, with the block allocated under the name tag (mem.c) */
i16 buffer_init_name(DosPort *port, u16 w, u16 h, char *tag)
{
    port->bits = mem_get_name((u32)h * w, tag);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}

/* buffer_free -- free the port's pixels and zero the record. Returns -1 when there were
 * pixels to free, otherwise 0.
 */
i16 buffer_free(DosPort *port)
{
    i16 freed;

    freed = 0;
    if (port->bits != 0) {
        mem_free(port->bits);
        freed = -1;
    }
    port->bits = 0;
    port->w = 0;
    port->h = 0;
    return freed;
}

/* buffer_draw_box -- the outline of the rectangle with corners (x0, y0) and (x1, y1), in
 * either order, clipped to the port
 */
void buffer_draw_box(i16 x0, i16 y0, i16 x1, DosPort port, i16 y1, char color)
{
    i16 t;

    if (x1 < x0) {
        t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 < y0) {
        t = y0;
        y0 = y1;
        y1 = t;
    }
    buffer_hline(x0, x1, y0, port, color);
    buffer_hline(x0, x1, y1, port, color);
    buffer_vline(x0, y0, y1, port, color);
    buffer_vline(x1, y0, y1, port, color);
}

/* buffer_conform -- clip the rectangle (*x, *y, *w, *h) to the port, in place. A negative x
 * or y moves to 0 and the width or height shrinks to match; the far edges are cut at the
 * port's. Returns 1 when nothing is left, else 0.
 */
i16 buffer_conform(DosPort *port, i16 *x, i16 *y, i16 *w, i16 *h)
{
    i16 x2;
    volatile i16 y2;

    if (*x < 0) {
        *w += *x;
        *x = 0;
    }
    if (*y < 0) {
        *h += *y;
        *y = 0;
    }
    x2 = min(*w + *x - 1, port->w - 1);
    y2 = min(*h + *y - 1, port->h - 1);
    *w = x2 - *x + 1;
    *h = y2 - *y + 1;
    if (*w <= 0 || *h <= 0)
        return 1;
    return 0;
}

static char g_swap_wb[] = "wb";          /* fopen modes for the swap files */
char buffer_disk_filename[] = "$MPOP.$";          /* swap file name; the slot digit follows */
u8 buffer_accum = 0;           /* the slot used last; the search for a free one starts after it */
u8 buffer_tracking[10] = { 0 };   /* per slot: 0xff in use, 0 free */
static char g_swap_rb[] = "rb";

/* buffer_to_disk -- write the rectangle (x, y, w, h) of the port, row by row, to the next free
 * swap file, taking slots round robin. Returns the slot (0-9), or -1 when all ten are in use or
 * the file cannot be written. The slot is marked in use before the file is opened and stays so
 * if the open or a write fails; a partly written file is left behind.
 */
i16 buffer_to_disk(DosPort *port, i16 x, i16 y, i16 w, i16 h)
{
    char name[40];
    i16 result;
    MSC_FILE *fp;
    i16 found;
    i16 tries;
    i16 i;

    result = -1;
    fp = 0;
    tries = 0;
    found = 0;
    do {
        buffer_accum = (u8)((buffer_accum + 1) % 10);
        msc_strcpy(name, buffer_disk_filename);
        env_catint(name, buffer_accum, 1);
        if (buffer_tracking[buffer_accum] == 0)
            found = -1;
        else if (++tries > 10)
            goto out;
    } while (!found);
    buffer_tracking[buffer_accum] = 0xff;
    fp = msc_fopen(name, g_swap_wb);
    if (fp) {
        for (i = 0; i < h; i++)
            if (fileio_fwrite_f(buffer_pointer(port, x, y + i), (i32)w, 1L, fp) == 0)
                goto out;
        result = buffer_accum;
    }
out:
    if (fp)
        msc_fclose(fp);
    return result;
}
/* buffer_from_disk -- read the rectangle (x, y, w, h) of the port back from swap file slot,
 * stopping at a short read. Unless keep is nonzero it then deletes the file and frees the
 * slot. When the file cannot be opened, the slot stays in use.
 */
void buffer_from_disk(DosPort *port, i16 slot, i16 keep, i16 x, i16 y, i16 w, i16 h)
{
    char name[40];
    MSC_FILE *fp;
    i16 i;

    msc_strcpy(name, buffer_disk_filename);
    env_catint(name, slot, 1);
    fp = msc_fopen(name, g_swap_rb);
    if (fp)
        for (i = 0; i < h; i++)
            if (fileio_fread_f(buffer_pointer(port, x, y + i), (i32)w, 1L, fp) == 0)
                break;
    if (fp) {
        msc_fclose(fp);
        if (!keep) {
            buffer_tracking[slot] = 0;
            msc_remove(name);
        }
    }
}

i16 buffer_restore_keep_flag = 0;          /* nonzero: buffer_restore keeps the saved copy */
static DosPort preserve_port;              /* the scratch port; holds one saved region */

/* buffer_preserve -- save the rectangle (x, y, w, h) of the port before something is drawn
 * over it, and return a handle for buffer_restore. The rectangle is first clipped to the port.
 * By mode:
 *   -8     copy it into the scratch port (a block named "$preserv"); if that cannot be
 *          allocated, write it to a swap file (buffer_to_disk);
 *   -2     save nothing;
 *   other  write it to a swap file.
 * The editor always passes -8. Returns -1 for the scratch port, -10 - slot for a swap file, or
 * -3 when nothing was saved (also when the clip leaves nothing). src is unused.
 * There is one scratch port: a second -8 save before the first is restored replaces it, and
 * the first block is never freed.
 */
i16 buffer_preserve(DosPort *port, i16 mode, i16 src, i16 x, i16 y, i16 w, i16 h)
{
    i16 res;

    res = -3;
    if (buffer_conform(port, &x, &y, &w, &h) != 0)
        goto out;
    if (mode == -8) {
        buffer_init_name(&preserve_port, w, h, "$preserv");
        if (preserve_port.bits != 0) {
            buffer_rect_copy_2(x, y, 0, *port, preserve_port, 0, w, h);
            res = -1;
            goto out;
        }
    }
    {
        if (mode != -2) {
            mode = buffer_to_disk(port, x, y, w, h);
            if (mode >= 0) {
                res = -10 - mode;
                goto out;
            }
        }
        res = -3;
    }
out:
    return res;
}
/* buffer_restore -- put back what buffer_preserve saved under handle, at the same (clipped)
 * rectangle: -1 copies the scratch port back and frees it, -10 to -19 read swap file
 * slot -10 - handle; -3 and any other handle do nothing. With buffer_restore_keep_flag set,
 * the saved copy is kept for another restore. dst is unused.
 */
void buffer_restore(DosPort *port, i16 handle, i16 dst, i16 x, i16 y, i16 w, i16 h)
{
    if (buffer_conform(port, &x, &y, &w, &h) != 0)
        return;
    switch (handle) {
    default:
        break;
    case -19: case -18: case -17: case -16: case -15:
    case -14: case -13: case -12: case -11: case -10:
        buffer_from_disk(port, ~(handle + 10) + 1, buffer_restore_keep_flag, x, y, w, h);
        break;
    case -3:
        return;
    case -1:
        buffer_rect_copy_2(0, 0, x, preserve_port, *port, y, w, h);
        if (buffer_restore_keep_flag == 0)
            buffer_free(&preserve_port);
        break;
    }
}

/* buffer_tile -- fill the rectangle (x, y, w, h) of dst with src repeated as a pattern whose
 * origin is at (sx, sy), as a backdrop. It works in bands of the pattern's height, each cut
 * into pieces of the pattern's width; every piece is one buffer_rect_copy_2, so nothing is
 * clipped to dst. An empty src draws nothing.
 * Quirk of the original: the phase is |x - sx| mod the pattern's width (likewise for y), so
 * when x < sx or y < sy the pattern starts at the wrong phase and does not line up with sx, sy.
 */
void buffer_tile(DosPort dst, DosPort src, i16 x, i16 y, i16 w, i16 h, i16 sx, i16 sy)
{
    i16 tw3;        /* the pattern's width */
    i16 th2;        /* the pattern's height */
    i16 next;       /* where the next piece of the row starts */
    i16 i;          /* where this piece starts */
    i16 oxc;        /* the source column of this piece (the phase, then 0) */
    i16 oy4;        /* the source row of this band (the phase, then 0) */
    i16 cy;         /* the top of this band */
    i16 u27;        /* x + w, before it is copied to right */
    i16 u18;        /* unused */
    i16 ox7;        /* the horizontal phase */
    i16 bottom9;    /* y + h */
    i16 right;      /* x + w */

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

/* buffer_pointer -- the address of pixel (x, y). w * y is an unsigned 32-bit product and x is
 * added unsigned, so a negative x or y addresses far past the pixels, not before them.
 */
char *buffer_pointer(DosPort *port, i16 x, i16 y)
{
    return port->bits + ((u32)(u16)port->w * (u16)y + (u16)x);
}

/* buffer_rect_fill -- fill w x h pixels at (x, y) with color, clipped to the port. Returns 1,
 * or 0 when the port has no pixels or the clip leaves nothing.
 */
i16 buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color)
{
    char *p;
    u16 r;
    if (buffer_conform(&port, &x, &y, &w, &h))
        return 0;               /* Port: the original returns an uninitialised value here */
    if (port.bits == 0)
        return 0;
    p = buffer_pointer(&port, x, y);
    for (r = (u16)h; r != 0; r--) {
        u16 i;
        for (i = 0; i < (u16)w; i++)
            p[i] = color;
        p += (u16)w + (i16)(port.w - w);
    }
    return 1;
}

/* copy_rect -- copy h rows of w bytes from s to d, skipping sskip and dskip bytes after each
 * row (the loop of buffer_rect_copy and buffer_rect_copy_2)
 */
static void copy_rect(char *d, const char *s, i16 w, i16 h, i16 sskip, i16 dskip)
{
    u16 r;
    for (r = (u16)h; r != 0; r--) {
        u16 i;
        for (i = 0; i < (u16)w; i++)
            d[i] = s[i];
        s += (u16)w + sskip;
        d += (u16)w + dskip;
    }
}

/* buffer_rect_copy -- copy w x h pixels at (x, y) of src to the same place in dst. No
 * clipping. Returns 1, or 0 (copying nothing) when either port has no pixels.
 */
i16 buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h)
{
    if (src.bits == 0 || dst.bits == 0)
        return 0;
    copy_rect(buffer_pointer(&dst, x, y), buffer_pointer(&src, x, y), w, h,
              (i16)(src.w - w), (i16)(dst.w - w));
    return 1;
}

/* buffer_rect_copy_2 -- buffer_rect_copy from (sx, sy) in src to (dx, dy) in dst */
i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h)
{
    if (src.bits == 0 || dst.bits == 0)
        return 0;
    copy_rect(buffer_pointer(&dst, dx, dy), buffer_pointer(&src, sx, sy), w, h,
              (i16)(src.w - w), (i16)(dst.w - w));
    return 1;
}

/* buffer_hline -- a horizontal line from x0 to x1 on row y, clipped to the port. x0 must not
 * be greater than x1. The pixel offset is computed in 16 bits.
 * Port: with x1 < x0 the original's count goes to 0 or below and it writes on to the end of
 * the 64 KB segment; the port draws nothing.
 */
void buffer_hline(i16 x0, i16 x1, i16 y, DosPort port, char color)
{
    i16 x;
    if (y < 0 || y >= port.h)
        return;
    if (x0 < 0)
        x0 = 0;
    if (x1 > port.w - 1)
        x1 = port.w - 1;
    for (x = x0; x <= x1; x++)
        port.bits[(u16)((u16)port.w * (u16)y + (u16)x)] = color;
}

/* buffer_vline -- a vertical line from y0 to y1 in column x, clipped to the port; nothing
 * when y1 < y0. The pixel offset is computed in 16 bits.
 */
void buffer_vline(i16 x, i16 y0, i16 y1, DosPort port, char color)
{
    i16 y;
    if (x < 0 || x >= port.w)
        return;
    if (y0 < 0)
        y0 = 0;
    if (y1 > port.h - 1)
        y1 = port.h - 1;
    for (y = y0; y <= y1; y++)
        port.bits[(u16)((u16)port.w * (u16)y + (u16)x)] = color;
}
