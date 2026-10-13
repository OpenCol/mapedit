/* From mapedit-decomp src/madsdev/ems_2 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE ems_2: MAPEDIT.EXE segment 1145 (ems_2.c), file 0x12a5c-0x12c86, 7 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn ems_activate_page_map 0x12a5c
 * fn ems_paging_setup      0x12a86
 * fn ems_free_page_handle  0x12af0
 * fn ems_get_page_handle   0x12b56
 * fn ems_next_handle_page  0x12c14
 * fn ems_paging_shutdown   0x12c66
 * fn ems_paging_mode       0x12c7a
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * EXPANDED MEMORY, THE PAGE ALLOCATOR (madsdev.lib). The last logical page is kept mapped
 * in physical page 1 as the "page flag" map: one byte per logical page, 0 when free,
 * handle + 1 when owned by a himem directory entry, 0xff for the map page itself.
 *   - ems_activate_page_map() maps that page in (once; ems_map_page(1, ..) resets
 *     ems_page_map_active) and returns 0 when it is mapped, 1 when the mapping failed.
 *   - ems_paging_setup() (needs ems_exists and at least two pages; 2048 at most) makes the
 *     page-flag map, counts the free pages in ems_pages_free and raises
 *     ems_paging_active; returns ems_paging_active.
 *   - ems_free_page_handle(handle) frees every page owned by handle and marks its entry
 *     in the EMS directory free (-1).
 *   - ems_get_page_handle(count) takes a directory entry (himem_resident(0)) and claims
 *     count free pages for it, or frees the entry again when it runs out; returns the
 *     handle (the entry number) or -1. The entry becomes kind 3 (EMS) with an empty name
 *     and byte 2 cleared. The page scan is `while (flag[i] != 0) if (++i >= pages) ...`:
 *     a do-while with `goto found` parks the failure block at the end.
 *   - ems_next_handle_page(handle, page) returns the first logical page owned by handle
 *     after page (-1 to start), or -1.
 *   - ems_paging_shutdown() unmaps everything and clears ems_paging_active.
 *   - ems_paging_mode(mode) sets how many free pages stay reserved
 *     (ems_paging_reserve[mode]; all four ship as 0).
 * The module's data (DS:0x4476-0x4487, CodeView names, shipped values) is defined before
 * the functions. Compiled with the global optimiser on (/Ox or /Oe, the pragma): the loop
 * counters live in bx/cx and globals are cached across the loops.
 */


#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    char kind;                  /* 3: in EMS, 4: in XMS, -1: free */
    char x1;
    u8 x2;           /* unsigned: the two zero stores below are immediates in
                                   source order; two chars share one `sub al,al` */
    char name[0x57];            /* +3: the file name himem_resident matches */
} HimemEntry;                   /* 0x5a bytes */

extern i16 ems_map_page(i16 physical, i16 logical);        /* 112d:00d4 */
extern void ems_unmap_all(void);                            /* 112d:015e */
extern i16 himem_resident(char *name);                  /* 1345:000a */
extern i16 ems_exists;                                      /* DS:0x443a (ems_1.c) */
extern u16 ems_pages;                                  /* DS:0x4440 (ems_1.c) */
extern i16 ems_page_map_active;                             /* DS:0x4448 (ems_1.c) */
extern char *ems_page[4];                               /* DS:0x445c (ems_1.c) */
extern HimemEntry *himem_ems_directory;                 /* DS:0x44fc (HIMEM_1.C) */
extern i16 himem_active;                                    /* DS:0x4504 (HIMEM_1.C) */
extern HimemEntry *himem_directory;                     /* DS:0x49c8 (HIMEM_1.C) */

i16 ems_pages_free = 0;                     /* DS:0x4476 */
i16 ems_pages_reserved = 0;                 /* DS:0x4478 */
i16 ems_paging_active = 0;                  /* DS:0x447a */
u8 *ems_page_flag = 0;       /* DS:0x447c */
i16 ems_paging_reserve[4] = {0, 0, 0, 0};   /* DS:0x4480 */

i16 ems_activate_page_map(void);
void ems_free_page_handle(i16 handle);
/* ems_activate_page_map -- 1145:000c, 42 bytes */
i16 ems_activate_page_map(void)
{
    if (ems_page_map_active == 0)
        ems_page_map_active = ems_map_page(1, ems_pages - 1) == 0;
    return ems_page_map_active == 0;
}
/* ems_paging_setup -- 1145:0036, 106 bytes */
/* PORT: ems_paging_setup is in port/ (inline assembly) */
i16 ems_paging_setup(void);

/* ems_free_page_handle -- 1145:00a0, 102 bytes */
void ems_free_page_handle(i16 handle)
{
    i16 i;

    if (ems_paging_active && himem_active && handle) {
        if (ems_activate_page_map() == 0) {
            for (i = 0; i < (i16)ems_pages; i++) {
                if (ems_page_flag[i] == (u8)(handle + 1)) {
                    ems_page_flag[i] = 0;
                    ems_pages_free++;
                }
            }
            himem_ems_directory[handle].kind = -1;
        }
    }
}
/* ems_get_page_handle -- 1145:0106, 190 bytes */
i16 ems_get_page_handle(i16 count)
{
    i16 i;
    i16 j;
    i16 handle;
    i16 result;

    result = -1;
    i = 0;
    if (ems_paging_active && himem_active && ems_pages_free - ems_pages_reserved >= count) {
        if (ems_activate_page_map() == 0) {
            handle = himem_resident(0);
            if (handle >= 0) {
                for (j = 0; j < count; j++) {
                    while (ems_page_flag[i] != 0) {
                        if (++i >= (i16)ems_pages) {
                            ems_free_page_handle(handle);
                            goto out;
                        }
                    }
                    ems_page_flag[i] = handle + 1;
                    ems_pages_free--;
                }
                result = handle;
                himem_directory[handle].kind = 3;
                himem_directory[handle].name[0] = 0;
                himem_directory[handle].x2 = 0;
            }
        }
    }
out:
    return result;
}
/* ems_next_handle_page -- 1145:01c4, 82 bytes */
i16 ems_next_handle_page(i16 handle, i16 page)
{
    i16 i;
    i16 result;

    result = -1;
    if (ems_paging_active) {
        if (ems_activate_page_map() == 0) {
            i = page + 1;
            do {
                if (ems_page_flag[i] == (u8)(handle + 1)) {
                    result = i;
                    break;
                }
                i++;
            } while (i < (i16)ems_pages);
        }
    }
    return result;
}
/* ems_paging_shutdown -- 1145:0216, 20 bytes */
void ems_paging_shutdown(void)
{
    if (ems_paging_active) {
        ems_unmap_all();
        ems_paging_active = 0;
    }
}
/* ems_paging_mode -- 1145:022a, 12 bytes */
void ems_paging_mode(i16 mode)
{
    ems_pages_reserved = ems_paging_reserve[mode];
}

