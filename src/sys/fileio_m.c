/* From mapedit-decomp src/madsdev/fileio_m (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "fileio.h"
#include "loader.h"
#include "units.h"
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

