/* Imported from mapedit-decomp src/madsdev/pack_9 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE pack_9: MAPEDIT.EXE segment 0f16 (pack_9.c), file 0x1076e-0x107aa, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_enable_pfab 0x1076e
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * pack_enable_pfab() switches the MADSPACK codec to the FAB compressor: strategy 1, the
 * enable flag, and the four pascal routines PFABCOMP and PFABEXP0..2 (pfab*.ASM) into
 * pack_5.c's routine pointers.
 */
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern char pack_pfab_enabled;                          /* DS:0x448b */
extern i16 pack_strategy;                               /* DS:0x448c */
extern void (*pack_pFABcomp_routine)(void);  /* DS:0x4496 */
extern void (*pack_pFABexp0_routine)(void);  /* DS:0x449a */
extern void (*pack_pFABexp1_routine)(void);  /* DS:0x449e */
extern void (*pack_pFABexp2_routine)(void);  /* DS:0x44a2 */
extern void pfabcomp(void);                      /* 1168:0006 */
extern void pfabexp0(void);                      /* 11a7:0000 */
extern void pfabexp1(void);                      /* 11a7:024e */
extern void pfabexp2(void);                      /* 11a7:045a */
/* pack_enable_pfab -- 0f16:000e, 60 bytes */
void pack_enable_pfab(void)
{
    pack_strategy = 1;
    pack_pfab_enabled = -1;
    pack_pFABcomp_routine = pfabcomp;
    pack_pFABexp0_routine = pfabexp0;
    pack_pFABexp1_routine = pfabexp1;
    pack_pFABexp2_routine = pfabexp2;
}

