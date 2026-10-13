/* From mapedit-decomp src/madsdev/buffer_t (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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


extern i16 buffer_conform(DosPort *port, i16 *x, i16 *y, i16 *w, i16 *h);
extern i16 buffer_init_name(DosPort *port, u16 w, u16 h, char *tag);
extern i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy,
                                        i16 w, i16 h);
extern i16 buffer_to_ems(DosPort *port, i16 handle, i16 src, i16 x, i16 y, i16 w, i16 h);
extern i16 buffer_to_disk(DosPort *port, i16 x, i16 y, i16 w, i16 h);
extern i16 buffer_from_ems(DosPort *port, i16 handle, i16 dst, i16 x, i16 y, i16 w, i16 h);
extern void buffer_from_disk(DosPort *port, i16 k, i16 keep, i16 x, i16 y, i16 w, i16 h);
extern i16 buffer_free(DosPort *port);

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
    res = buffer_to_ems(port, mode, src, x, y, w, h);
    if (res < 0) {
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
    default:
        if (buffer_restore_keep_flag != 0)
            handle &= 0xbfff;
        buffer_from_ems(port, handle, dst, x, y, w, h);
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

