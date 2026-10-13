/* Imported from mapedit-decomp src/madsdev/xms_3 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_3: MAPEDIT.EXE segment 1281 (XMS_3.C), file 0x13e18-0x13e90, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_umb_get  0x13e18
 * fn xms_umb_free 0x13e5a
 *
 * No VICEROY twin: reconstructed from the shipped bytes (VICEROY.EXE has the same bytes in
 * its hand-written assembly, frames 1103 and 1107).
 *
 * UPPER MEMORY BLOCKS (madsdev.lib). The module keeps the segments of the UMBs it handed
 * out in xms_umb_list, xms_umb_mark of them (at most 16).
 *   - xms_umb_get(bytes) asks the XMS driver for a UMB of (bytes - 1) / 16 + 1 paragraphs
 *     (function 10h) and returns it as a far pointer, segment:0, or NULL when the list is
 *     full or the driver refuses. The new segment is recorded at xms_umb_list[2 * mark]
 *     (the `shl bx,2` is the shipped library's: the list is walked with a word stride).
 *   - xms_umb_free(p) removes p's segment from the list, closing the gap, and releases the
 *     block (function 11h). The return is whatever the driver left in ax.
 * Both are inline assembly. xms_umb_mark (DS:0x44f6) is the module's one initialised
 * global; xms_umb_list (DS:0x5aac) is communal.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void (*xms_controller)(void);    /* DS:0x3c6c (xms_1.c) */

i16 xms_umb_mark = 0;                       /* DS:0x44f6 */
u16 xms_umb_list[16];                  /* DS:0x5aac */
/* xms_umb_get -- 1281:0008, 66 bytes */
/* PORT: xms_umb_get is in port/ (inline assembly) */
void *xms_umb_get(i32 bytes);

/* xms_umb_free -- 1281:004a, 54 bytes */
/* PORT: xms_umb_free is in port/ (inline assembly) */
i16 xms_umb_free(void *p);


