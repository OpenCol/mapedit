/* From mapedit-decomp src/madsdev/ems_5 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE ems_5: MAPEDIT.EXE segment 12fb (EMS_5.C), file 0x145b6-0x1462a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn ems_map_buffer 0x145b6
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * MAPPING A HANDLE'S FIRST FOUR PAGES (madsdev.lib). ems_map_buffer(handle) finds the
 * first four logical pages owned by handle (ems_next_handle_page from -1) and maps them
 * into physical pages 0-3, so the 64K buffer is addressable at ems_page[0]. 0 on
 * success, -1 without paging or when a mapping fails (the rest stay unmapped). The early
 * exit is `goto out` (an inline epilogue copy); a second `return status;` is merged into it.
 */

extern i16 ems_map_page(i16 physical, i16 logical);            /* 112d:00d4 */
extern i16 ems_next_handle_page(i16 handle, i16 page); /* 1145:01c4 */
extern i16 ems_paging_active;                                   /* DS:0x447a (ems_2.c) */
/* ems_map_buffer -- 12fb:0006, 116 bytes */
i16 ems_map_buffer(i16 handle)
{
    i16 p;              /* the logical page last found */
    i16 status;
    i16 i;
    i16 map[4];         /* the handle's first four logical pages */

    p = status = -1;
    if (ems_paging_active) {
        for (i = 0; i < 4; i++) {
            p = ems_next_handle_page(handle, p);
            map[i] = p;
        }
        for (i = 0; i < 4; i++) {
            if (ems_map_page(i, map[i]) != 0)
                goto out;
        }
        status = 0;
    }
out:
    return status;
}

