/* Imported from mapedit-decomp src/madsdev/xms_1 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_1: MAPEDIT.EXE segment 0f36 (xms_1.c), file 0x1096a-0x109ec, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_detect   0x1096a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_108c).
 *
 * LOOKING FOR XMS (root code past the C runtime, frame 108c). xms_detect() asks int 2Fh for
 * an XMS driver (4300h), takes its entry point (4310h) and its version (function 0), and
 * sets xms_exists when the version is 2.00 or later -- unless g_xms_off is set, which main
 * (ovl25_70c6a.c) does first thing, so the probe always fails in the shipped game. It then
 * resets three state words and, with XMS, takes a one-byte block through 1103:000a (xms_umb_get) to
 * remember the paragraph below it. Nothing calls it (no thunk, no direct call). The probe,
 * the three resets and the segment arithmetic are `_asm` (their immediate stores are not
 * MSC's: it would load -1 into ax once for both words); the allocation is C.
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern i16 xms_exists;                     /* DS:0x26e4 */
extern i16 xms_disabled;                  /* DS:0x26e6: main sets it, so no XMS */
extern void (*xms_controller)(void);    /* DS:0x26e8 */
extern u16 xms_chain_seg;              /* DS:0x26ec */
extern i32 umb_max_free;                /* DS:0x2666 (ovl31_78856) */
extern i16 pad_108c_1;                   /* keeps the symbol count of the two word names it replaces */
extern char umb_max_free_set;              /* DS:0x264e (ovl31_78856) */
extern u16 xms_version;           /* DS:0xa668 */
extern void *xms_umb_get(i32 n);
extern void xms_umb_free(void *p);
/* xms_detect -- 0f36:000a, 130 bytes */
/* PORT: xms_detect is in port/ (inline assembly) */
i16 xms_detect(void);


/* DGROUP data (DS:0x26e4-0x26ef): the XMS state xms_detect fills, in address order after the last function (so no
 * function's symbol count moves; the externs above stay as they were). Shipped values. */
i16 xms_exists = 0;                                   /* DS:0x26e4 */
i16 xms_disabled = 0;                                /* DS:0x26e6 */
void (*xms_controller)(void) = 0;                  /* DS:0x26e8 */
u16 xms_chain_seg = 0x0000;                       /* DS:0x26ec */
u16 xms_preloaded = 0x0000;                           /* DS:0x26ee */

