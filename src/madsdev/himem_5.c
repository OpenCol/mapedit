/* Imported from mapedit-decomp src/madsdev/himem_5 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE himem_5: MAPEDIT.EXE segment 1297 (himem_5.c), file 0x13f7e-0x140a0, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn himem_directory_setup 0x13f7e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * MAKING THE HIGH-MEMORY DIRECTORY (madsdev.lib). himem_directory_setup() places the
 * 150-entry directory (0x34bc bytes): with EMS paging it is the top half of physical
 * page 1 (allocation 3, himem_ems_directory = ems_page[1] + 0x800, behind the page-flag
 * map), otherwise an XMS block (allocation 4, himem_directory_xms_handle) built in a
 * temporary conventional block "$HIMEM$" and copied up, with himem_directory_entry
 * pointing at the save area that himem_get_directory_entry reads entries into. Every
 * entry starts free (kind -1). On success himem_active is raised and 0 returned; on any
 * failure the allocation is -1 again, an XMS handle is released, and -1 returned. The
 * temporary block is freed either way.
 * Compiled with the global optimiser on (/Ox or /Oe): see the pragma.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

// MSC: #pragma optimize("elg", on)

typedef struct {
    char kind;                  /* 3: in EMS, 4: in XMS, -1: free */
    char pad[0x59];
} HimemEntry;                   /* 0x5a bytes */

extern i16 himem_activate_directory(void);                  /* 1290:0006 */
extern i16 xms_get(i32 bytes);                   /* 128b:0004 */
extern void xms_free(i16 handle);                 /* 128b:0040 */
extern void *mem_get_name(i32 size, char *tag); /* 0cc9:0136 */
extern i16 mem_free(void *p);                 /* 0cc9:0310 */
extern i16 xms_copy(i32 length, i16 src_handle, i32 src_offset, i16 dst_handle, i32 dst_offset); /* 1343:0004 */
extern i16 ems_paging_active;                               /* DS:0x447a (ems_2.c) */
extern char *ems_page[4];                               /* DS:0x445c (ems_1.c) */
extern char himem_preload_xms_disabled;                     /* DS:0x44f9 (HIMEM_1.C) */
extern char himem_directory_allocation;                     /* DS:0x44fa (HIMEM_1.C) */
extern HimemEntry *himem_ems_directory;                 /* DS:0x44fc (HIMEM_1.C) */
extern i16 himem_directory_xms_handle;                      /* DS:0x4500 (HIMEM_1.C) */
extern i16 himem_active;                                    /* DS:0x4504 (HIMEM_1.C) */
extern HimemEntry *himem_directory;                     /* DS:0x49c8 (HIMEM_1.C) */
extern HimemEntry *himem_directory_entry;               /* DS:0x4b56 (HIMEM_1.C) */
extern HimemEntry himem_directory_save_area;                /* DS:0x648e (HIMEM_1.C) */
/* himem_directory_setup -- 1297:000e, 290 bytes */
i16 himem_directory_setup(void)
{
    i16 result;
    i32 size;
    HimemEntry *block;
    i16 n;              /* the clearing loop: MSC's loop optimiser turns the
                           indexed store into a `loop` over a strength-reduced
                           pointer; a pointer-walking spelling comes out 6 bytes off */

    result = -1;
    size = 0;
    block = 0;
    if (ems_paging_active) {
        himem_directory_allocation = 3;
        himem_ems_directory = (HimemEntry *)(ems_page[1] + 0x800);
        himem_directory = himem_ems_directory;
        if (himem_activate_directory() != 0)
            goto fail;
    } else {
        if (himem_preload_xms_disabled)
            goto fail;
        himem_directory_allocation = 4;
        size = 150 * 0x5aL;
        himem_directory_xms_handle = xms_get(size);
        if (himem_directory_xms_handle <= 0)
            goto fail;
        block = mem_get_name(size, "$HIMEM$");
        himem_directory = block;
        if (block == 0)
            goto fail;
        himem_directory_entry = &himem_directory_save_area;
    }
    for (n = 0; n < 150; n++)
        himem_directory[n].kind = -1;
    if (himem_directory_allocation == 4) {
        if (xms_copy(size, 0, (i32)block, himem_directory_xms_handle, 0L) != 0)
            goto fail;
    }
    himem_active = -1;
    result = 0;
fail:
    if (block)
        mem_free(block);
    if (result != 0) {
        himem_directory_allocation = -1;
        if (himem_directory_xms_handle > 0) {
            xms_free(himem_directory_xms_handle);
            himem_directory_xms_handle = -1;
        }
    }
    return result;
}

