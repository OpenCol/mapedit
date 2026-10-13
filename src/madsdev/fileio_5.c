/* Imported from mapedit-decomp src/madsdev/fileio_5 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE fileio_5: MAPEDIT.EXE segment 0bc4 (FILEIO_5.C), file 0xd240-0xd2a0, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_fix_lf_input  0x0d240
 * fn fileio_fix_lf_output 0x0d26e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0afb).
 *
 * NEWLINES IN TEXT LINES (root code, frame 0afb; pascal, "elg").
 *   - fileio_fix_lf_input(s) ends s at its last '\n' and returns where that was (0 if none).
 *   - fileio_fix_lf_output(s) appends "\n".
 */

extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

// MSC: #pragma optimize("elg", on)
/* fileio_fix_lf_input -- 0bc4:0000, 46 bytes */
char * fileio_fix_lf_input(char *s)
{
    char *p;

    p = msc__fstrrchr(s, '\n');
    if (p)
        *p = 0;
    return p;
}
/* fileio_fix_lf_output -- 0bc4:002e, 50 bytes */
void fileio_fix_lf_output(char *s)
{
    char *t;

    t = s + msc__fstrlen(s);
    *t++ = '\n';
    *t = 0;
}

