/* Imported from mapedit-decomp src/madsdev/himem_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE himem_2: MAPEDIT.EXE segment 1292 (HIMEM_2.C), file 0x13f2a-0x13f7e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn himem_get_directory_entry 0x13f2a
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * SELECTING A DIRECTORY ENTRY (madsdev.lib). himem_get_directory_entry(n) makes entry n of
 * the high-memory directory addressable through himem_directory_entry: with an EMS
 * directory it maps the directory in and points at the entry; otherwise it copies the
 * entry (0x5a bytes at n * 0x5a) out of the XMS directory into
 * himem_directory_save_area, which himem_directory_setup pointed himem_directory_entry
 * at. 0 on success, -1 when the mapping or the copy failed.
 * Compiled with the global optimiser on (/Ox or /Oe): see the pragma.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

/* The global register allocator: n goes straight to di and result to si with no frame
 * (plain /Ot homes n to the stack). Any of "e", "el", "eg", "elg", "aelg" matches. */
// MSC: #pragma optimize("elg", on)

typedef struct {
    char kind;                  /* 3: in EMS, 4: in XMS, -1: free */
    char pad[0x59];
} HimemEntry;                   /* 0x5a bytes */

extern i16 himem_activate_directory(void);                  /* 1290:0006 */
extern i16 xms_copy(i32 length, i16 src_handle, i32 src_offset, i16 dst_handle, i32 dst_offset); /* 1343:0004 */
extern char himem_directory_allocation;                     /* DS:0x44fa (HIMEM_1.C) */
extern HimemEntry *himem_ems_directory;                 /* DS:0x44fc (HIMEM_1.C) */
extern i16 himem_directory_xms_handle;                      /* DS:0x4500 (HIMEM_1.C) */
extern HimemEntry *himem_directory_entry;               /* DS:0x4b56 (HIMEM_1.C) */
extern HimemEntry himem_directory_save_area;                /* DS:0x648e (HIMEM_1.C) */
/* himem_get_directory_entry -- 1292:000a, 84 bytes */
i16 himem_get_directory_entry(i16 n)
{
    register i16 result;

    result = -1;
    if (himem_directory_allocation == 3) {
        if (himem_activate_directory() == 0) {
            himem_directory_entry = himem_ems_directory + n;
            result = 0;
        }
    } else {
        if (xms_copy(0x5aL, himem_directory_xms_handle, n * 0x5aL, 0,
                     (i32)(void *)&himem_directory_save_area) == 0)
            result = 0;
    }
    return result;
}

