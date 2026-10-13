/* Imported from mapedit-decomp src/madsdev/buffer_s by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE buffer_s: MAPEDIT.EXE segment 103d (buffer_s.c), file 0x119da-0x11af2, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn buffer_to_ems   0x119da
 * fn buffer_from_ems 0x11a6e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * SAVING A SCREEN PORT TO EMS. The EMS page frame (ems_page, a far pointer) is treated as a
 * 320x200 port.
 *   - buffer_to_ems(handle, src, port, x, y, w, h) (_fastcall) copies the rectangle of
 *     `port` into the page: handle < 0 allocates 4 pages (and marks the returned handle
 *     with 0x4000 so buffer_from_ems frees it); src < 0 copies from the port through
 *     buffer_rect_copy_2, else from another EMS buffer `src`. Returns the handle (or the
 *     allocation failure) after unmapping.
 *   - buffer_from_ems(handle, dst, port, x, y, w, h) (_fastcall) is the reverse: dst < 0
 *     copies the page's rectangle back into the port, else into EMS buffer `dst`; a
 *     handle marked 0x4000 is freed. Returns the (unmarked) handle.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct { i16 h; i16 w; char *bits; } DosPort;

extern i16 ems_get_page_handle(i16 pages);
extern void ems_map_buffer(i16 handle);
extern void ems_buffer_to_buffer(i16 from, i16 to);
extern void ems_unmap_all(void);
extern void ems_free_page_handle(i16 handle);
extern i16 buffer_rect_copy_2(i16 sx, i16 sy, i16 dx, DosPort src, DosPort dst, i16 dy, i16 w, i16 h);
extern char *ems_page;                       /* DS:0x445c */
/* buffer_to_ems -- 103d:000a, 148 bytes */
i16 buffer_to_ems(i16 handle, i16 src, DosPort *port, i16 x, i16 y, i16 w, i16 h)
{
    i16 mark;
    DosPort page;

    mark = 0;
    page.h = 200;
    page.w = 320;
    if (handle < 0) {
        handle = ems_get_page_handle(4);
        mark = 0x4000;
    }
    if (handle >= 0) {
        if (src < 0) {
            ems_map_buffer(handle);
            page.bits = ems_page;
            buffer_rect_copy_2(x, y, x, *port, page, y, w, h);
        } else {
            ems_buffer_to_buffer(src, handle);
        }
        handle |= mark;
        ems_unmap_all();
    }
    return handle;
}
/* buffer_from_ems -- 103d:009e, 132 bytes */
i16 buffer_from_ems(i16 handle, i16 dst, DosPort *port, i16 x, i16 y, i16 w, i16 h)
{
    i16 mark;
    DosPort page;

    page.h = 200;
    page.w = 320;
    mark = handle & 0x4000;
    handle &= ~0x4000;
    if (dst < 0) {
        ems_map_buffer(handle);
        page.bits = ems_page;
        buffer_rect_copy_2(x, y, x, page, *port, y, w, h);
    } else {
        ems_buffer_to_buffer(handle, dst);
    }
    if (mark)
        ems_free_page_handle(handle);
    ems_unmap_all();
    return handle;
}

