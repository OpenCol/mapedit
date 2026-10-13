/* From mapedit-decomp src/madsdev/btype_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE btype_3: MAPEDIT.EXE segment 1050 (btype_3.c), file 0x11b0a-0x11b6e, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xtoi         0x11b0a
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_109a).
 *
 * HEX TEXT TO INT (root code past the C runtime, frame 109a; _fastcall). xtoi(p) is the
 * Windows/Mac xtoi (`xtoi`) with near text and <ctype.h>'s macros for its
 * toupper and isdigit: stoi (root_0c06.c) calls it for "0x..." numbers. A character that is
 * not a hex digit ends the number (the rest of the text is skipped).
 */

#include "msc.h"
/* xtoi -- 1050:000a, 100 bytes */
i16 xtoi(char *p)
{
    i16 result = 0;
    i16 ch;

    while (*p != 0) {
        ch = *p;
        p++;
        ch = toupper(ch);
        if (isdigit(ch)) {
            result = result << 4;
            result += ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            result = result << 4;
            result += ch - 0x37;
        } else {
            while (*p != 0)
                p++;
        }
    }
    return result;
}

