/* From mapedit-decomp src/madsdev/himem_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE himem_1: MAPEDIT.EXE segment 1290 (HIMEM_1.C), file 0x13f06-0x13f2a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn himem_activate_directory 0x13f06
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * THE HIGH-MEMORY DIRECTORY'S STATE (madsdev.lib). The "himem" layer keeps a directory of
 * 150 entries (0x5a bytes each, 0x34bc in all) describing the files it preloaded into EMS
 * or XMS. himem_directory_allocation says where the directory itself lives: 3 in EMS (it is
 * the second EMS page, himem_ems_directory), 4 in XMS (handle himem_directory_xms_handle,
 * read one entry at a time into himem_directory_save_area), -1 nowhere.
 *   - himem_activate_directory() makes the directory addressable: for an EMS directory it
 *     maps the page set in (ems_activate_page_map) and records whether that worked in
 *     himem_directory_ems_active. It returns 0 when the directory can be used, 1 when the
 *     mapping failed; an XMS or absent directory needs nothing and gives 0.
 * The module defines the state (DS:0x44f8-0x4505, shipped values) and the communal
 * directory pointer, the current-entry pointer and the save area.
 */

typedef struct {
    char kind;                  /* 3: in EMS, 4: in XMS, -1: free */
    char pad[0x59];
} HimemEntry;                   /* 0x5a bytes */

extern i16 ems_activate_page_map(void);           /* 1145:000c */

char himem_preload_ems_disabled = 0;        /* DS:0x44f8 */
char himem_preload_xms_disabled = 0;        /* DS:0x44f9 */
char himem_directory_allocation = -1;       /* DS:0x44fa */
HimemEntry *himem_ems_directory = 0;    /* DS:0x44fc */
i16 himem_directory_xms_handle = -1;        /* DS:0x4500 */
i16 himem_directory_ems_active = 0;         /* DS:0x4502 */
i16 himem_active = 0;                       /* DS:0x4504 */
HimemEntry *himem_directory;            /* DS:0x49c8 */
HimemEntry *himem_directory_entry;      /* DS:0x4b56 */
HimemEntry himem_directory_save_area;       /* DS:0x648e */
/* himem_activate_directory -- 1290:0006, 36 bytes */
i16 himem_activate_directory(void)
{
    register i16 failed;

    failed = 0;
    if (himem_directory_allocation == 3) {
        himem_directory_ems_active = ems_activate_page_map() == 0;
        failed = himem_directory_ems_active == 0;
    }
    return failed;
}

