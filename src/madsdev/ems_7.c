/* From mapedit-decomp src/madsdev/ems_7 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE ems_7: MAPEDIT.EXE segment 1302 (ems_7.c), file 0x1462a-0x1469a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn ems_buffer_to_buffer 0x1462a
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * COPYING ONE HANDLE'S BUFFER TO ANOTHER'S (madsdev.lib). ems_buffer_to_buffer(src, dst)
 * walks the first four logical pages of each handle in step, maps the source page into
 * physical page 2 and the destination page into physical page 3, and copies the 16K
 * between them with _fmemcpy. Nothing is checked.
 */

extern i16 ems_map_page(i16 physical, i16 logical);            /* 112d:00d4 */
extern i16 ems_next_handle_page(i16 handle, i16 page); /* 1145:01c4 */
extern char *ems_page[4];                                   /* DS:0x445c (ems_1.c) */
/* ems_buffer_to_buffer -- 1302:000a, 112 bytes */
void ems_buffer_to_buffer(i16 src_handle, i16 dst_handle)
{
    i16 src_page;
    i16 dst_page;
    i16 i;

    dst_page = src_page = -1;
    for (i = 0; i < 4; i++) {
        src_page = ems_next_handle_page(src_handle, src_page);
        dst_page = ems_next_handle_page(dst_handle, dst_page);
        ems_map_page(2, src_page);
        ems_map_page(3, dst_page);
        msc__fmemcpy(ems_page[3], ems_page[2], 0x4000);
    }
}

