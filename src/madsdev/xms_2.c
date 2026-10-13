/* Imported from mapedit-decomp src/madsdev/xms_2 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_2: MAPEDIT.EXE segment 127e (XMS_2.C), file 0x13dec-0x13e18, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_umb_get_avail 0x13dec
 *
 * No VICEROY twin: reconstructed from the shipped bytes (VICEROY.EXE has the same bytes in
 * its hand-written assembly, frame 1100).
 *
 * THE LARGEST FREE UPPER MEMORY BLOCK (madsdev.lib). xms_umb_get_avail() asks the XMS
 * driver for an impossible UMB (function 10h, DX = 0FFFFh paragraphs); the driver refuses
 * with BL = B0h and DX = the largest block it could give, which comes back as bytes
 * (paragraphs * 16 as a long). No driver, or any other refusal, is 0.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i16 xms_exists;                      /* DS:0x3c68 (xms_1.c) */
extern void (*xms_controller)(void);    /* DS:0x3c6c (xms_1.c) */
/* xms_umb_get_avail -- 127e:000c, 44 bytes */
/* PORT: xms_umb_get_avail is in port/ (inline assembly) */
i32 xms_umb_get_avail(void);


