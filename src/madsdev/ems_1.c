/* Imported from mapedit-decomp src/madsdev/ems_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE ems_1: MAPEDIT.EXE segment 112d (ems_1.c), file 0x128d6-0x12a5c, 6 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn ems_detect    0x128d6
 * fn ems_shutdown  0x1298a
 * fn ems_map_page  0x129a4
 * fn ems_push      0x129e4
 * fn ems_pop       0x12a06
 * fn ems_unmap_all 0x12a2e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * EXPANDED MEMORY, THE DRIVER LEVEL (madsdev.lib). The game maps 16K EMS pages into the
 * four physical pages of the page frame (ems_page[0..3] are their far addresses).
 *   - ems_detect() looks for the EMM: the int 67h vector's segment must carry the device
 *     name "EMMXXXX0", ems_disabled must be clear, the driver must report status 0 (40h)
 *     and version 3.0 or later (46h, BCD: ems_high_version.ems_low_version). It takes the
 *     page frame segment (41h) and fills ems_page, counts the free pages (42h) and
 *     allocates them all under one handle (43h). Returns -1 (ems_exists) when it has the
 *     pages, 0 otherwise; ems_driver is -1 as soon as a usable driver is found. Inline
 *     assembly throughout, with three C return points. Each is `goto out;` to the final
 *     `return;`: written as three `return;`s, MSC cross-jumps the identical
 *     `mov ax,bx` + epilogue tails into one and jumps to it, where shipped has three copies.
 *   - ems_shutdown() releases the handle (45h) and clears ems_exists.
 *   - ems_map_page(physical, logical) maps a logical page into physical page 0-3 (44h)
 *     unless it is already there (ems_page_mapped), returns the driver's status in ah or 0
 *     when nothing had to change, and notes the change in ems_mapping_changed. Mapping
 *     physical page 1 invalidates ems_page_map_active (ems_2.c's page-flag map lives there).
 *   - ems_push() / ems_pop() save the four mappings in ems_page_stack and restore them.
 *   - ems_unmap_all() unmaps the four physical pages (logical -1), on EMS 4.0 or later.
 * The module's data (DS:0x4438-0x4475, CodeView names, shipped values) is defined before
 * the functions.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

i16 ems_driver = 0;                         /* DS:0x4438: -1 once a usable EMM is found */
i16 ems_exists = 0;                         /* DS:0x443a: -1 once the pages are allocated */
u16 ems_page_frame = 0;                /* DS:0x443c */
i16 ems_handle = 0;                         /* DS:0x443e */
i16 ems_pages = 0;                          /* DS:0x4440 */
u16 ems_high_version = 0;              /* DS:0x4442 */
i16 ems_low_version = 0;                    /* DS:0x4444 */
i16 ems_disabled = 0;                       /* DS:0x4446 */
i16 ems_page_map_active = 0;                /* DS:0x4448 */
i16 ems_mapping_changed = -1;               /* DS:0x444a */
i16 ems_page_mapped[4] = {-1, -1, -1, -1};  /* DS:0x444c */
i16 ems_page_stack[4] = {-1, -1, -1, -1};   /* DS:0x4454 */
char *ems_page[4] = {0, 0, 0, 0};       /* DS:0x445c */
char ems_name[] = "EMMXXXX0";               /* DS:0x446c */

i16 ems_map_page(i16 physical, i16 logical);
/* ems_detect -- 112d:0006, 180 bytes */
/* PORT: ems_detect is in port/ (inline assembly) */
i16 ems_detect(void);

/* ems_shutdown -- 112d:00ba, 26 bytes */
/* PORT: ems_shutdown is in port/ (inline assembly) */
void ems_shutdown(void);

/* ems_map_page -- 112d:00d4, 64 bytes */
/* PORT: ems_map_page is in port/ (inline assembly) */
i16 ems_map_page(i16 physical, i16 logical);

/* ems_push -- 112d:0114, 34 bytes */
void ems_push(void)
{
    i16 i;

    for (i = 0; i < 4; i++)
        ems_page_stack[i] = ems_page_mapped[i];
}
/* ems_pop -- 112d:0136, 40 bytes */
void ems_pop(void)
{
    i16 i;

    for (i = 0; i < 4; i++)
        ems_map_page(i, ems_page_stack[i]);
}
/* ems_unmap_all -- 112d:015e, 46 bytes */
void ems_unmap_all(void)
{
    i16 i;

    if (ems_exists && ems_high_version >= 4) {
        for (i = 0; i < 4; i++)
            ems_map_page(i, -1);
    }
}

