/* Imported from mapedit-decomp src/madsdev/fileio_u by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE fileio_u: MAPEDIT.EXE segment 0c1a (FILEIO_U.C), file 0xd7a2-0xd810, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_purge_all_spaces 0x0d7a2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0b57).
 *
 * TRIMMING A LINE (root code, frame 0b57; pascal, "elg"). fileio_purge_all_spaces(s) trims
 * trailing blanks (fileio_purge_trailing_spaces, 1094:000a, which walks back over spaces and tabs) and then
 * leading spaces and tabs, through a 256-byte copy. The skip loop runs with DS on s's
 * segment and reloads DGROUP afterwards: MSC's code for a far pointer walked in a loop.
 */

// MSC: #pragma optimize("elg", on)
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;

extern void fileio_purge_trailing_spaces(char *s);
/* fileio_purge_all_spaces -- 0c1a:0002, 110 bytes */
void fileio_purge_all_spaces(char *s)
{
    char buf[256];
    char *p;

    fileio_purge_trailing_spaces(s);
    p = s;
    while (*p && (*p == ' ' || *p == '\t'))
        p++;
    msc__fstrcpy(buf, p);
    msc__fstrcpy(s, buf);
}

