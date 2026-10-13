/* buffer.c -- the MADS buffer library: 2D byte arrays (a DosPort: height, width, bits), the pixel and rectangle primitives on them, and saving what a window covers. */
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
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */

/* MODULE buffer_3: MAPEDIT.EXE segment 0c49 (BUFFER_3.C), file 0xda9e-0xdac0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_fill  0x0da9e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b8d).
 *
 * FILLING A WHOLE PORT (root code, frame 0b8d). buffer_fill(c, port) (_fastcall, the
 * colour in al) fills the port with c.
 */



/* buffer_fill -- 0c49:000e, 34 bytes */
void buffer_fill(char c, DosPort port)
{
    buffer_rect_fill(0, 0, port.w, port, port.h, c);
}

/* MODULE buffer_7: MAPEDIT.EXE segment 0c76 (buffer_7.c), file 0xdd64-0xdd80, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_put   0x0dd64
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bb9).
 *
 * PUTTING ONE BYTE INTO A PORT (root code, frame 0bb9). buffer_put(x, y, v, port) (_fastcall).
 */



/* buffer_put -- 0c76:0004, 28 bytes */
void buffer_put(i16 x, i16 y, i16 v, DosPort port)
{
    *buffer_pointer(&port, x, y) = (char)v;
}

/* MODULE buffer_8: MAPEDIT.EXE segment 0c78 (buffer_8.c), file 0xdd80-0xdd96, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_get   0x0dd80
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bbb).
 *
 * READING ONE BYTE FROM A PORT (root code, frame 0bbb). buffer_get(x, y, port) (_fastcall).
 */



/* buffer_get -- 0c78:0000, 22 bytes */
u8 buffer_get(i16 x, i16 y, DosPort port)
{
    return *buffer_pointer(&port, x, y);
}

/* MODULE buffer_1: MAPEDIT.EXE segment 0c3e (buffer_1.c), file 0xd9ea-0xda66, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_init      0x0d9ea
 * fn buffer_init_name 0x0da24
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl31_787a2).
 *
 * CREATING A DRAWING PORT (overlay 31, para 22). DOS-only engine code; both functions are
 * _fastcall, with the port in bx, w in ax and h in dx.
 *   - buffer_init allocates w * h bytes as one far block (181f:029a) for port->bits, and
 *     fills in w and h. It returns -1, or 0 when the allocation fails.
 *   - buffer_init_name does the same through the tagged allocator (181f:2e90), under the
 *     tag string the caller passes on the stack.
 */



/* buffer_init -- 0c3e:000a, 58 bytes */
i16 buffer_init(DosPort *port, u16 w, u16 h)
{
    port->bits = mem_get((u32)h * w);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}
/* buffer_init_name -- 0c3e:0044, 66 bytes */
i16 buffer_init_name(DosPort *port, u16 w, u16 h, char *tag)
{
    port->bits = mem_get_name((u32)h * w, tag);
    if (port->bits == 0)
        return 0;
    port->w = w;
    port->h = h;
    return -1;
}

/* MODULE buffer_2: MAPEDIT.EXE segment 0c46 (BUFFER_2.C), file 0xda66-0xda9e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_free  0x0da66
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/ovl31_7881e).
 *
 * FREEING A DRAWING PORT (overlay 31, para 29). DOS-only engine code; _fastcall, with the
 * port in bx. It returns -1
 * when it freed something, otherwise 0.
 */



/* buffer_free -- 0c46:0006, 56 bytes */
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

/* MODULE buffer_4: MAPEDIT.EXE segment 0c4c (buffer_4.c), file 0xdac0-0xdbb4, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_copy 0x0dac0
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b8f).
 *
 * COPYING A RECTANGLE BETWEEN PORTS (root code, frame 0b8f). buffer_rect_copy(x, y, w, src, dst, h)
 * (_fastcall) copies (x, y, w, h) from src to the same place in dst when both have pixels
 * and returns whether it did. The rows are copied in inline assembly: word moves, an odd
 * byte, and a 32 KB segment step on each side.
 */



/* buffer_rect_copy -- 0c4c:0000, 244 bytes */
/* PORT: buffer_rect_copy is in port/ (inline assembly) */

/* MODULE buffer_5: MAPEDIT.EXE segment 0c5b (BUFFER_5.C), file 0xdbb4-0xdc70, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_fill 0x0dbb4
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b9e).
 *
 * FILLING A RECTANGLE (root code, frame 0b9e). buffer_rect_fill(x, y, w, port, h, color)
 * (_fastcall) clips to the port (buffer_conform), then fills the rows in inline assembly: word
 * stores, an odd byte, and a 32 KB segment step. It returns whether the port had pixels;
 * after a clip that leaves nothing, the return value is uninitialised (shipped behaviour).
 */



/* buffer_rect_fill -- 0c5b:0004, 188 bytes */
/* PORT: buffer_rect_fill is in port/ (inline assembly) */

/* MODULE buffer_6: MAPEDIT.EXE segment 0c67 (buffer_6.c), file 0xdc70-0xdd64, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_rect_copy_2 0x0dc70
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0baa).
 *
 * COPYING A RECTANGLE BETWEEN PORTS (root code, frame 0baa). buffer_rect_copy_2(sx, sy, dx, src,
 * dst, dy, w, h) (_fastcall) is port_blit (root_0b8f.c) with separate source and destination
 * corners. The rows are copied in inline assembly: word moves, an odd
 * byte, and a 32 KB segment step on each side.
 */



/* buffer_rect_copy_2 -- 0c67:0000, 244 bytes */
/* PORT: buffer_rect_copy_2 is in port/ (inline assembly) */

/* MODULE buffer_9: MAPEDIT.EXE segment 0c79 (buffer_9.c), file 0xdd96-0xde00, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_hline 0x0dd96
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bbc).
 *
 * A HORIZONTAL LINE (root code, frame 0bbc). buffer_hline(x0, x1, y, port, color) (_fastcall) clips
 * to the port in C, then draws in inline assembly. Compare vline (root_0bc3.c), whose
 * loop does not add the port's offset.
 */


/* buffer_hline -- 0c79:0006, 106 bytes */
/* PORT: buffer_hline is in port/ (inline assembly) */

/* MODULE buffer_a: MAPEDIT.EXE segment 0c80 (buffer_a.c), file 0xde00-0xde6c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_vline 0x0de00
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bc3).
 *
 * A VERTICAL LINE (root code, frame 0bc3). buffer_vline(x, y0, y1, port, color) (_fastcall) clips
 * to the port in C, then draws in inline assembly.
 */


/* buffer_vline -- 0c80:0000, 108 bytes */
/* PORT: buffer_vline is in port/ (inline assembly) */

/* MODULE buffer_b: MAPEDIT.EXE segment 0c86 (buffer_b.c), file 0xde6c-0xdf10, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_draw_box 0x0de6c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bca).
 *
 * A RECTANGLE OUTLINE (root code, frame 0bca). buffer_draw_box(x0, y0, x1, port, y1, color)
 * (_fastcall) orders the corners, then draws two hlines and two vlines.
 */



/* buffer_draw_box -- 0c86:000c, 164 bytes */
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



/* buffer_pointer -- 0c91:0000, 68 bytes */
/* PORT: buffer_pointer is in port/ (inline assembly) */

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

/* MODULE buffer_r: MAPEDIT.EXE segment 1025 (buffer_r.c), file 0x1185c-0x119da, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_to_disk   0x1185c
 * fn buffer_from_disk 0x1193a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0bdd).
 *
 * SAVING WHAT A WINDOW COVERS TO DISK (root code, frame 0bdd). Backs port_save_under /
 * port_restore_under (ovl31) when there is no memory for a scratch port.
 *   - buffer_to_disk(port, x, y, w, h) (_fastcall) takes the next free one of ten slots
 *     (round robin), writes the rectangle row by row to "$MPOP.$<slot>", and returns the
 *     slot, or -1.
 *   - buffer_from_disk(port, slot, keep, x, y, w, h) (_fastcall) reads it back, then deletes
 *     the file and frees the slot unless `keep`.
 */




static char g_swap_wb[] = "wb";          /* DS:0x43e8: a module static (no public) */
char buffer_disk_filename[] = "$MPOP.$";          /* DS:0x2634 */
u8 buffer_accum = 0;           /* DS:0x263c */
u8 buffer_tracking[10] = { 0 };   /* DS:0x263e */
static char g_swap_rb[] = "rb";          /* DS:0x4400: a module static (no public) */
/* buffer_to_disk -- 1025:000c, 222 bytes */
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
/* buffer_from_disk -- 1025:00ea, 160 bytes */
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

/* MODULE buffer_t: MAPEDIT.EXE segment 0c9d (buffer_t.c), file 0xdfde-0xe190, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 * (this file was built with /Oe as well: #pragma optimize("e", on) at the top, like buffer_s.c)
 *
 * fn buffer_preserve 0x0dfde
 * fn buffer_restore  0x0e0bc
 *
 * Twin: `ovl31_78640` port_save_under / port_restore_under (VICEROY.EXE), the
 * same source with the editor's library names and an EMS path the game's build lacks.
 *
 * SAVING AND RESTORING WHAT A WINDOW COVERS. Both are _fastcall with the port in bx, a
 * mode or handle in ax, a second int in dx, and the rectangle x, y, w, h on the stack
 * (pushed in that order: a by-value {h, w, y, x} record compiles the same, but the callers
 * pass four ints, and a modern compiler passes the two differently).
 *   - buffer_preserve(port, mode, src, x, y, w, h) clips the rectangle to the port (buffer_conform
 *     adjusts it in place) and answers -3 when nothing is left. Mode -8 copies the region
 *     into the module's scratch port (buffer_init_name under the tag "$preserv") and
 *     answers -1. Otherwise the region goes to EMS (buffer_to_ems, whose handle >= 0 is
 *     the answer); failing that, unless the mode is -2, to disk (buffer_to_disk), a
 *     k >= 0 from it answered as -10 - k; else -3.
 *   - buffer_restore(port, handle, dst, x, y, w, h) takes that handle back: -3 does nothing, -1
 *     blits the scratch port back to (x, y) and frees it unless buffer_restore_keep_flag
 *     is set, -19..-10 passes k = -10 - handle (and the flag) to buffer_from_disk, and any
 *     other handle goes to buffer_from_ems (with bit 0x4000 cleared when the flag is set,
 *     so the EMS pages are kept).
 *
 * What the bytes pinned:
 *   - /Oe. Under plain /Ot the twin's spellings (`DosPort * volatile port`, `register`
 *     parameters) get within 2 bytes of buffer_restore (the si/di copy order) and 30 of
 *     buffer_preserve, whose `sub si,-10; neg si` negates the mode IN PLACE: no plain-flags
 *     spelling negates a register variable in place (every one gives `mov ax,si; neg ax`),
 *     while under "e" the plain `res = -10 - mode;` is exactly that. With "e" on, no
 *     `register` or `volatile` is needed in either function; the parameters are plain.
 *   - The restore switch is written with its default arm FIRST, which puts the
 *     subtract-chain dispatch at the top with the EMS arm right after it.
 *   - `~(handle + 10) + 1` for buffer_from_disk's lea/not/inc (the twin's spelling).
 */



i16 buffer_restore_keep_flag = 0;          /* DS:0x746 */
static DosPort preserve_port;              /* DS:0x499e (this module's _BSS) */

/* buffer_preserve -- 0c9d:000e, 222 bytes */
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
    /* (1994 tried EMS first, which the port does not have) */
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
/* buffer_restore -- 0c9d:00ec, 212 bytes */
void buffer_restore(DosPort *port, i16 handle, i16 dst, i16 x, i16 y, i16 w, i16 h)
{
    if (buffer_conform(port, &x, &y, &w, &h) != 0)
        return;
    switch (handle) {
    default:                    /* an EMS handle: the port never gives one out */
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

/* buffer.c -- madsdev's work-buffer primitives, whose inner loops MicroProse wrote in
 * assembly (BUFFER_3/4/5/6/9/A.C, buffer_i.c). C equivalents of the shipped code, byte for
 * byte in effect: tests/oracle runs the original beside each of them.
 *
 * A DosPort's pixels were a huge block: the original walks it with far pointers it keeps
 * normalised (offset below 8000h, the segment advanced 800h at a time). A flat pointer needs
 * none of that; what is kept is every pixel each routine writes, and the order.
 */

/* buffer_pointer -- 0c91:0000: the address of pixel (x, y). The original multiplies w * y
   unsigned (MUL, 16 x 16 -> 32 bits) and adds x as an unsigned word (ADD, ADC 0). */
char *buffer_pointer(DosPort *port, i16 x, i16 y)
{
    return port->bits + ((u32)(u16)port->w * (u16)y + (u16)x);
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
