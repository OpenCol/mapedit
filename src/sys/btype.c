/* btype.c -- numbers in text: binary, decimal and hexadecimal. */
#include "msc.h"
#include "btype.h"
#include "font.h"
#include "loader.h"
#include "popup.h"
#include "units.h"
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

/* MODULE btype_5: MAPEDIT.EXE segment 0d12 (btype_5.c), file 0xe72c-0xe786, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn stoi         0x0e72c
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c06).
 *
 * NUMBERS IN TEXT FILES (root code, frame 0c06). stoi(s) (_fastcall) reads "0x..." and
 * "0b..." through xtoi and btoi (root_109a.c, root_10a0.c), "0d..." and anything
 * else through atoi.
 */


/* stoi -- 0d12:000c, 90 bytes */
i16 stoi(char *s)
{
    char c;

    if (s[0] == '0') {
        c = s[1];
        if ((_ctype + 1)[(u8)c] & 2)
            c -= 0x20;
        if (c == 'X')
            return xtoi(s + 2);
        if (c == 'B')
            return btoi(s + 2);
        if (c == 'D')
            return msc_atoi(s + 2);
    }
    return msc_atoi(s);
}
