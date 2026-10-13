/* Imported from mapedit-decomp src/madsdev/fileio_i by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE fileio_i: MAPEDIT.EXE segment 0c04 (FILEIO_I.C), file 0xd64e-0xd714, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_add_ext 0x0d64e
 * fn fileio_new_ext 0x0d69c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b32).
 *
 * FILE NAME EXTENSIONS (root code, frame 0b32; pascal, "elg").
 *   - fileio_add_ext(name, ext) appends "." and ext when name has no '.', then upper-cases it.
 *   - fileio_new_ext(dst, src, ext) copies src to dst (unless they are the same), drops
 *     anything from the last '.', appends "." and ext, and upper-cases the result.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

/* fileio_add_ext -- 0c04:000e, 78 bytes */
void fileio_add_ext(char *name, char *ext)
{
    if (!msc__fstrrchr(name, '.')) {
        msc__fstrcat(name, ".");
        msc__fstrcat(name, ext);
    }
    msc__fstrupr(name);
}
/* fileio_new_ext -- 0c04:005c, 120 bytes */
void fileio_new_ext(char *dst, char *src, char *ext)
{
    char *p;

    if (dst != src)
        msc__fstrcpy(dst, src);
    p = msc__fstrrchr(dst, '.');
    if (p)
        *p = 0;
    msc__fstrcat(dst, ".");
    msc__fstrcat(dst, ext);
    msc__fstrupr(dst);
}

