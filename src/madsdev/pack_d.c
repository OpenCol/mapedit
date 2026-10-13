/* From mapedit-decomp src/madsdev/pack_d (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pack_d: MAPEDIT.EXE segment 1378 (PACK_D.C), file 0x14d84-0x14e8e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_write_ems 0x14d84
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * PACK_WRITE_EMS(buf, n) is a MADSPACK writer callback (pascal, like the FFIO type in
 * pack_6.c) that stores decoded data into EMS: it takes min(*n, pack_write_size) bytes
 * (pack_write_size < 0 means no limit) and copies them 16 KB page by 16 KB page, moving
 * to the handle's next page (ems_next_handle_page) when the current one is full, mapping
 * it at physical page 0 and copying into the page frame. It counts the bytes into
 * pack_write_count and off pack_write_size, and returns the byte count it took.
 */
#define min(a, b) ((a) < (b) ? (a) : (b))

extern i32 pack_write_size;                            /* DS:0x4a84 */
extern char *ems_page;                              /* DS:0x445c */
extern i16 pack_ems_page_offset;                        /* DS:0x4e82 */
extern i16 pack_ems_page_marker;                        /* DS:0x4bb4 */
extern i16 pack_ems_page_handle;                        /* DS:0x6978 */
extern i32 pack_write_count;                           /* DS:0x6984 */
extern i16 ems_next_handle_page(i16 handle, i16 page);   /* 1145:01c4 */
extern void ems_map_page(i16 phys, i16 page);           /* 112d:00d4 */
extern void *mem_normalize(void *p);  /* 104f:0002 */
/* pack_write_ems -- 1378:0004, 266 bytes */
i16 pack_write_ems(char *buf, u16 *n)
{
    char *p9;
    i16 count;
    i16 left;
    i16 chunk4;
    char *page0;

    if (pack_write_size >= 0)
        count = (i16)min((i32)*n, pack_write_size);
    else
        count = *n;
    if (count != 0) {
        left = count;
        p9 = buf;
        page0 = ems_page;
        do {
            if (pack_ems_page_offset >= 0x4000) {
                pack_ems_page_marker = ems_next_handle_page(pack_ems_page_handle, pack_ems_page_marker);
                pack_ems_page_offset = 0;
                if (pack_ems_page_marker < 0)
                    break;
            }
            chunk4 = min(0x4000 - pack_ems_page_offset, left);
            ems_map_page(0, pack_ems_page_marker);
            msc__fmemcpy(page0 + pack_ems_page_offset, p9, chunk4);
            p9 = mem_normalize(p9 + chunk4);
            left -= chunk4;
            pack_ems_page_offset += chunk4;
            if (pack_write_size > 0)
                pack_write_size -= (u16)chunk4;
            pack_write_count += (u16)chunk4;
        } while (left != 0);
    }
    return count;
}

/* The module's uninitialised data (communals, BSS). */
i16 pack_ems_page_marker;                 /* DS:0x4bb4 */
i16 pack_ems_page_offset;                 /* DS:0x4e82 */
i16 pack_ems_page_handle;                 /* DS:0x6978 */

