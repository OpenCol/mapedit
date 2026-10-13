/* buffer.c -- madsdev's work-buffer primitives, whose inner loops MicroProse wrote in
 * assembly (BUFFER_3/4/5/6/9/A.C, buffer_i.c). C equivalents of the shipped code, byte for
 * byte in effect: tests/oracle runs the original beside each of them.
 *
 * A DosPort's pixels were a huge block: the original walks it with far pointers it keeps
 * normalised (offset below 8000h, the segment advanced 800h at a time). A flat pointer needs
 * none of that; what is kept is every pixel each routine writes, and the order.
 */
#include "msc.h"
#include "menu.h"
#include "pal.h"
#include "popup.h"
#include "sprite.h"
#include "buffer.h"
#include "mem.h"
#include "records.h"
#include "units.h"
#include "port_records.h"


/* buffer_pointer -- 0c91:0000: the address of pixel (x, y). The original multiplies w * y
   unsigned (MUL, 16 x 16 -> 32 bits) and adds x as an unsigned word (ADD, ADC 0). */
char *buffer_pointer(DosPort *port, i16 x, i16 y)
{
    return port->bits + ((u32)(u16)port->w * (u16)y + (u16)x);
}

/* mem_normalize -- 104f:0002: a far pointer with the smallest offset. Flat pointers are
   normal already. */
char *mem_normalize(char *p)
{
    return p;
}

/* buffer_rect_fill -- 0c5b:0004: fill w x h pixels at (x, y) with color, after clipping
   (buffer_conform). Returns whether the buffer has pixels -- when the clip leaves nothing,
   the original returns the uninitialised local it would have set, here 0. */
i16 buffer_rect_fill(i16 x, i16 y, i16 w, DosPort port, i16 h, char color)
{
    char *p;
    u16 r;
    if (buffer_conform(&port, &x, &y, &w, &h))
        return 0;               /* PORT: the original returns a stack slot's leftover here */
    if (port.bits == 0)
        return 0;
    p = buffer_pointer(&port, x, y);
    /* rows of w bytes (STOSW pairs, then the odd one), each followed by a skip of port.w - w;
       h == 0 writes nothing */
    for (r = (u16)h; r != 0; r--) {     /* DEC SI; JNZ: a 16-bit count */
        u16 i;
        for (i = 0; i < (u16)w; i++)
            p[i] = color;
        p += (u16)w + (i16)(port.w - w);
    }
    return 1;
}

/* the shared copy loop of buffer_rect_copy and buffer_rect_copy_2 */
static void copy_rect(char *d, const char *s, i16 w, i16 h, i16 sskip, i16 dskip)
{
    u16 r;
    for (r = (u16)h; r != 0; r--) {     /* DEC AX; JNZ: a 16-bit count */
        u16 i;
        for (i = 0; i < (u16)w; i++)
            d[i] = s[i];
        s += (u16)w + sskip;
        d += (u16)w + dskip;
    }
}

/* buffer_rect_copy -- 0c4c:0000: copy w x h pixels at (x, y) from src to the same place in
   dst. No clipping. Returns whether both buffers have pixels. */
i16 buffer_rect_copy(i16 x, i16 y, i16 w, DosPort src, DosPort dst, i16 h)
{
    if (src.bits == 0 || dst.bits == 0)
        return 0;
    copy_rect(buffer_pointer(&dst, x, y), buffer_pointer(&src, x, y), w, h,
              (i16)(src.w - w), (i16)(dst.w - w));
    return 1;
}

/* buffer_rect_copy_2 -- 0c67:0000: the same, from (sx, sy) in src to (dx, dy) in dst */
i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h)
{
    if (src.bits == 0 || dst.bits == 0)
        return 0;
    copy_rect(buffer_pointer(&dst, dx, dy), buffer_pointer(&src, sx, sy), w, h,
              (i16)(src.w - w), (i16)(dst.w - w));
    return 1;
}

/* buffer_hline -- 0c79:0006: a horizontal line from x0 to x1 on row y, clipped to the
   buffer. The original's loop is LOOPNZ over x1 - x0 + 1 bytes: with x1 < x0 the count is
   zero or negative and it runs on to the end of the 64 KB segment. The port draws nothing
   past x1 (PORT). */
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

/* buffer_vline -- 0c80:0000: a vertical line from y0 to y1 in column x, clipped */
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
