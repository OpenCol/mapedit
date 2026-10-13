/* From mapedit-decomp src/madsdev/fileio_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "popup.h"
#include "fileio.h"
#include "loader.h"
#include "records.h"
#include "units.h"
/* MODULE fileio_1: MAPEDIT.EXE segment 101f (FILEIO_1.C), file 0x117fc-0x1185c, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn fileio_purge_trailing_spaces 0x117fc
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1094).
 *
 * TRIMMING TRAILING BLANKS (root code past the C runtime, frame 1094; pascal, "el": with /Og
 * MSC turns the end test into `dec di; cmp di,s`, shipped keeps p and tests `lea ax,[di-1]`).
 * fileio_purge_trailing_spaces(s) cuts spaces and tabs off the end of s, one per pass, re-measuring s each
 * time, and stops at the first other character or at the start of s. Its first _fstrlen
 * is a dead length: the call stays, the store does not.
 */


/* fileio_purge_trailing_spaces -- 101f:000c, 96 bytes */
void fileio_purge_trailing_spaces(char *s)
{
    char *p;
    i16 len;
    i16 more;

    len = msc__fstrlen(s);
    if (len == 0)
        return;     /* 1994 looked at the byte before an empty string; nothing to purge */
    do {
        more = -1;
        p = &s[msc__fstrlen(s) - 1];
        if (*p == ' ' || *p == '\t')
            *p = 0;
        else
            more = 0;
        if (p - 1 < s)
            more = 0;
    } while (more);
}

