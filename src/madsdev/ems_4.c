/* From mapedit-decomp src/madsdev/ems_4 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE ems_4: MAPEDIT.EXE segment 12ec (EMS_4.C), file 0x144c8-0x145b6, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn ems_copy_it_down 0x144c8
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * READING A HANDLE'S PAGES DOWN INTO CONVENTIONAL MEMORY (madsdev.lib).
 * ems_copy_it_down(handle, &page, &offset, dst, count) copies count bytes of the handle's
 * data, starting at *offset into logical page *page, to dst: each page is mapped into
 * physical page 2 and copied with _fmemcpy, the next page of the handle follows
 * (ems_next_handle_page) when the offset reaches 16K, and the destination pointer is
 * normalised after every chunk (mem_check_overflow). *page and *offset are left where
 * the copy stopped, so the next call carries on. 0 when count is exhausted, -1 when a
 * mapping fails or the handle runs out of pages.
 */

#define min(a, b) ((a) <= (b) ? (a) : (b))

extern i16 ems_map_page(i16 physical, i16 logical);            /* 112d:00d4 */
extern i16 ems_next_handle_page(i16 handle, i16 page); /* 1145:01c4 */
extern void *mem_check_overflow(void *p);     /* 1357:000e */
extern char *ems_page[4];                                   /* DS:0x445c (ems_1.c) */
/* ems_copy_it_down -- 12ec:0008, 238 bytes */
i16 ems_copy_it_down(i16 handle, i16 *page, i16 *offset, char *dst, i32 count)
{
    u16 n;         /* count -= n zero-extends: unsigned */
    i16 len;            /* unused; the shipped frame has a third word (bucket 15, before result) */
    i16 result;

    result = -1;
    if (*page >= 0) {
        if (ems_map_page(2, *page) != 0)
            goto out;
    }
    while (count > 0) {
        if (*offset >= 0x4000) {
            *page = ems_next_handle_page(handle, *page);
            if (*page < 0)
                goto out;
            if (ems_map_page(2, *page) != 0)
                goto out;
            *offset = 0;
        }
        n = (i16)min((i32)(u16)(0x4000 - *offset), count);
        if (n != 0) {
            msc__fmemcpy(dst, ems_page[2] + *offset, n);
            dst += n;
            *offset += n;
            count -= n;
        }
        dst = mem_check_overflow(dst);
    }
    result = 0;
out:
    return result;
}

