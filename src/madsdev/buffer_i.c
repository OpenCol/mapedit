/* Imported from mapedit-decomp src/madsdev/buffer_i by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_i: MAPEDIT.EXE segment 0c91 (buffer_i.c), file 0xdf10-0xdfde, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_pointer 0x0df10
 * fn buffer_conform 0x0df54
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0a4e).
 * buffer_pointer has no VICEROY twin: reconstructed from the shipped bytes.
 *
 * PORT GEOMETRY (root code, frame 0a4e).
 *   - buffer_pointer(port, x, y) (_fastcall) is the far address of pixel (x, y), NORMALISED
 *     (offset 0..15, the rest in the segment) in inline assembly: y * w + x as a 32-bit
 *     product, carried into the segment. VICEROY's port_pixel is the plain C sum.
 *   - buffer_conform(port, &x, &y, &w, &h) (_fastcall, thunk 181f:0ecc) clips a rectangle to the
 *     port in place and returns 1 when nothing is left.
 *
 * Plain house flags (port_pixel matches only without /Oe). Without
 *     it MSC holds y2 in cx and the function is 6 bytes short; with it on x2 instead, 33
 *     bytes differ;
 *   - the exit written as `if (*w <= 0 || *h <= 0) return 1; return 0;`, so return 1 is the
 *     fall-through and return 0 the out-of-line copy. `&&` with `return 0` first lays the two
 *     epilogues out the other way round, and two separate ifs cost 2 bytes.
 * Pads 0..16 and swapping the x2/y2 declarations change nothing either way. The
 *     shipped code loads the segment as a RELOCATED IMMEDIATE, not from ds
 *     (`mov dx,DGROUP; mov ax,0x5cc; retf` = 7 bytes); a near `extern char g_05cc[]` cast
 *     to `char far *` gives `mov ax,offset; mov dx,ds` = 6. Declaring the array itself
 *     `far` is what buys the immediate -- the variable is still at DS:0x05cc, but `far`
 *     makes MSC emit `mov dx,seg g_05cc` instead of `mov dx,ds`, which is the shipped
 *     instruction and the shipped length. What is left is only the ORDER: MSC always loads
 *     the offset into ax first and the segment into dx second, the shipped code the other
 *     way round, so the two B8/BA opcode bytes are swapped. No spelling moves it (`&a[0]`,
 *     `a + 0`, a `(void far *)` cast, a far local, `huge`), and a 32-bit constant cast to
 *     `char far *` is ax-first too and loses the fixup mask (6 bytes differ). dx-first is
 *     what a hand-written stub does; compare the one at 0xc8db in module 0a0a. The only way to get dx first is `_asm { mov
 *     dx, SEG g; mov ax, OFFSET g }`, which gives exactly the two shipped instructions with
 *     the right fixups -- but any function containing `_asm` gets a `push bp; mov bp,sp ...
 *     leave` frame (`_fastcall`, pascal, `#pragma optimize("", off)` included: that one also
 *     saves si/di), 11 bytes for the shipped 7. The 7-byte frameless stub is assembled.
 */

#define min(a, b) ((a) <= (b) ? (a) : (b))
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern char g_05cc[];                /* DS:0x05cc */
/* buffer_pointer -- 0c91:0000, 68 bytes */
/* PORT: buffer_pointer is in port/ (inline assembly) */
char *buffer_pointer(DosPort *port, i16 x, i16 y);

/* buffer_conform -- 0c91:0044, 138 bytes */
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

