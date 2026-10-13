/* Imported from mapedit-decomp src/madsdev/btype_4 by tools/import.py. Do not edit; mark a
 * deliberate change with PORT: (docs/plan.md). */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE btype_4: MAPEDIT.EXE segment 1056 (btype_4.c), file 0x11b6e-0x11bb6, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn btoi         0x11b6e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_10a0).
 *
 * BINARY TEXT TO INT (root code past the C runtime, frame 10a0; _fastcall). btoi(p) is the
 * Windows/Mac btoi (`btoi`) with near text and <ctype.h>'s toupper macro; stoi
 * (root_0c06.c) calls it for "0b..." numbers.
 */

// MSC: #pragma optimize("elg", on)
#include "msc.h"
extern i16 stand_in_1, stand_in_2, stand_in_3, stand_in_4, stand_in_5, stand_in_6;
extern i16 stand_in_7, stand_in_8, stand_in_9, stand_in_10, stand_in_11, stand_in_12;
/* btoi -- 1056:000e, 72 bytes */
i16 btoi(char *p)
{
    i16 value = 0;
    i16 c;

    while (*p != 0) {
        c = *p;
        p++;
        c = toupper(c);
        if (c == '0' || c == '1') {
            value = value * 2;
            value += c - '0';
        } else {
            while (*p != 0)
                p++;
        }
    }
    return value;
}

