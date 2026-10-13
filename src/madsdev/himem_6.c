/* Imported from mapedit-decomp src/madsdev/himem_6 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE himem_6: MAPEDIT.EXE segment 0f3e (himem_6.c), file 0x109ec-0x10a50, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn himem_shutdown 0x109ec
 * fn himem_startup  0x10a3a
 *
 * HIGH-MEMORY STARTUP AND SHUTDOWN (madsdev.lib). The "himem" layer keeps a directory of
 * up to 150 preloaded files in EMS or XMS (HIMEM_1.C's himem_directory_entry points at the
 * entry himem_get_directory_entry selected).
 *   - himem_shutdown: when the directory can be activated, it walks all 150 entries and
 *     frees the XMS handle of every entry kept in XMS (kind 4). Then it frees the
 *     directory's own XMS handle if the directory lives in XMS, clears himem_active,
 *     purges the UMB list and shuts EMS down.
 *   - himem_startup detects EMS, sets EMS paging up, detects XMS and sets the directory up.
 * The module has no data of its own.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

typedef struct {
    char kind;                  /* 4: in XMS */
    char pad[0x11];
    i16 handle;                 /* +0x12: the XMS handle */
} HimemEntry;

extern i16 himem_activate_directory(void);                  /* 1290:0006 */
extern void himem_get_directory_entry(i16 n);     /* 1292:000a */
extern void xms_free(i16 handle);                 /* 128b:0040 */
extern void xms_umb_purge(void);                            /* 1289:0000 */
extern void ems_shutdown(void);                   /* 112d:00ba */
extern HimemEntry *himem_directory_entry;               /* DS:0x4b56 */
extern char himem_directory_allocation;                     /* DS:0x44fa */
extern i16 himem_directory_xms_handle;                      /* DS:0x4500 */
extern i16 himem_active;                                    /* DS:0x4504 */
extern void ems_detect(void);                     /* 112d:0006 */
extern void ems_paging_setup(void);               /* 1145:0036 */
extern void xms_detect(void);                               /* 0f36:000a */
extern void himem_directory_setup(void);                    /* 1297:000e */
/* himem_shutdown -- 0f3e:000c, 78 bytes */
void himem_shutdown(void)
{
    register i16 i;

    if (himem_activate_directory() == 0) {
        for (i = 0; i < 150; i++) {
            himem_get_directory_entry(i);
            if (himem_directory_entry->kind == 4)
                xms_free(himem_directory_entry->handle);
        }
    }
    if (himem_directory_allocation == 4)
        xms_free(himem_directory_xms_handle);
    himem_active = 0;
    xms_umb_purge();
    ems_shutdown();
}
/* himem_startup -- 0f3e:005a, 22 bytes */
void himem_startup(void)
{
    ems_detect();
    ems_paging_setup();
    xms_detect();
    himem_directory_setup();
}

