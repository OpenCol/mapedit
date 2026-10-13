/* From mapedit-decomp src/madsdev/keys_3 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "keys.h"
#include "loader.h"
#include "units.h"
/* MODULE keys_3: MAPEDIT.EXE segment 0bb2 (keys_3.c), file 0xd12e-0xd240, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn keys_fix_alt 0x0d12e
 *
 * Twin: `root_0aea` alt_letter (same bytes; "elg").
 *
 * ALT+LETTER KEYS. keys_fix_alt(key) (_fastcall) maps an Alt+letter key code (0x100 + the
 * scan code) to its upper-case letter, and any other key to itself with a-z upper-cased.
 */
/* keys_fix_alt -- 0bb2:000e, 274 bytes */
i16 keys_fix_alt(i16 key)
{
    switch (key) {
    case 0x110:
        return 'Q';
    case 0x111:
        return 'W';
    case 0x112:
        return 'E';
    case 0x113:
        return 'R';
    case 0x114:
        return 'T';
    case 0x115:
        return 'Y';
    case 0x116:
        return 'U';
    case 0x117:
        return 'I';
    case 0x118:
        return 'O';
    case 0x119:
        return 'P';
    case 0x11e:
        return 'A';
    case 0x11f:
        return 'S';
    case 0x120:
        return 'D';
    case 0x121:
        return 'F';
    case 0x122:
        return 'G';
    case 0x123:
        return 'H';
    case 0x124:
        return 'J';
    case 0x125:
        return 'K';
    case 0x126:
        return 'L';
    case 0x12c:
        return 'Z';
    case 0x12d:
        return 'X';
    case 0x12e:
        return 'C';
    case 0x12f:
        return 'V';
    case 0x130:
        return 'B';
    case 0x131:
        return 'N';
    case 0x132:
        return 'M';
    default:
        if (key >= 'a' && key <= 'z')
            return key - 0x20;
        return key;
    }
}

