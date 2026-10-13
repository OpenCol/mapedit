/* From mapedit-decomp src/madsdev/btype_5 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
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


extern u8 _ctype[];           /* DS:0x27ec: the runtime's character classes; ctype.h indexes _ctype+1 */
extern i16 xtoi(char *s);
extern i16 btoi(char *s);
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

