/* From mapedit-decomp src/madsdev/pack_9 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "pack.h"
#include "records.h"
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

