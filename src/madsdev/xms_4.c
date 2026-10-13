/* Imported from mapedit-decomp src/madsdev/xms_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_4: MAPEDIT.EXE segment 1289 (XMS_4.C), file 0x13e90-0x13eb4, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_umb_purge 0x13e90
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * RELEASING EVERY UPPER MEMORY BLOCK (madsdev.lib). xms_umb_purge() walks the segments
 * XMS_3.C recorded in xms_umb_list (xms_umb_mark of them), releases each through the XMS
 * driver (function 11h, DX = segment) and empties the list. himem_shutdown calls it.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void (*xms_controller)(void);    /* DS:0x3c6c (xms_1.c) */
extern i16 xms_umb_mark;                    /* DS:0x44f6 (XMS_3.C) */
extern u16 xms_umb_list[16];           /* DS:0x5aac (XMS_3.C) */
/* xms_umb_purge -- 1289:0000, 36 bytes */
/* PORT: xms_umb_purge is in port/ (inline assembly) */
void xms_umb_purge(void);


