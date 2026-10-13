/* Imported from mapedit-decomp src/madsdev/fileio_m by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE fileio_m: MAPEDIT.EXE segment 0c11 (FILEIO_M.C), file 0xd714-0xd7a2, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_join_path 0x0d714
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b4e).
 *
 * JOINING A PATH (root code, frame 0b4e; pascal, "elg"). fileio_join_path(dst, dir, name) copies
 * dir, adds a '\' unless it already ends with one, appends name, upper-cases the result in
 * dst and returns dst.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

/* fileio_join_path -- 0c11:0004, 142 bytes */
char * fileio_join_path(char *dst, char *dir, char *name)
{
    char buf[80];
    char *p;

    msc__fstrcpy(buf, dir);
    for (p = buf; *p; p++)
        ;
    if (p[-1] != '\\')
        msc_strcat(buf, "\\");
    msc__fstrcpy(dst, buf);
    msc__fstrcat(dst, name);
    msc__fstrupr(dst);
    return dst;
}

