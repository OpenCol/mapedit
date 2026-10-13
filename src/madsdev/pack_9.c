/* From mapedit-decomp src/madsdev/pack_9 (f55aff8), now maintained here: docs/origin.md. */
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

extern char pack_pfab_enabled;                          /* DS:0x448b */
extern i16 pack_strategy;                               /* DS:0x448c */
typedef u16 (*FFIO)(void *buf, u16 *n);
typedef i16 (*FFPACK)(FFIO rd, FFIO wr, void *work, i16 *type, u16 *dsize);
typedef i16 (*FFUNPACK)(FFIO rd, FFIO wr, void *work);
typedef i16 (*FFUNPACK_TO)(FFIO rd, void *dst, void *work);
typedef i16 (*FFUNPACK_MEM)(void *src, void *dst, void *work);
extern FFPACK pack_pFABcomp_routine;                    /* DS:0x4496 (pack_5.c's types) */
extern FFUNPACK pack_pFABexp0_routine;                  /* DS:0x449a */
extern FFUNPACK_TO pack_pFABexp1_routine;               /* DS:0x449e */
extern FFUNPACK_MEM pack_pFABexp2_routine;              /* DS:0x44a2 */
extern i16 pfabcomp(FFIO rd, FFIO wr, void *work, i16 *type, u16 *dsize); /* 1168:0006 */
extern i16 pfabexp0(FFIO rd, FFIO wr, void *work);                 /* 11a7:0000 */
extern i16 pfabexp1(FFIO rd, void *dst, void *work);           /* 11a7:024e */
extern i16 pfabexp2(void *src, void *dst, void *work);     /* 11a7:045a */
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

