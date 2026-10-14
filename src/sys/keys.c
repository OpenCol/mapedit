/* keys.c -- the keyboard: the BIOS queue.
 *
 * keys_any and keys_get read the keyboard queue, which in 1994 was the BIOS's (INT 16h). A key
 * comes back as one number: its ASCII code, or 100h + its scan code for a key without one
 * (arrows, function keys, Alt+letter). keys_fix_alt turns Alt+letter into the letter.
 */
#include "msc.h"
#include "host.h"
#include "font.h"
#include "keys.h"
#include "loader.h"
#include "machine.h"
#include "mcga.h"
#include "menu.h"
#include "mouse.h"
#include "popup.h"
#include "records.h"
#include "timer.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */
#include "video.h"

/* keys_fix_alt -- maps an Alt+letter key (100h + its scan code) to the upper-case letter, and
 * any other key to itself with a-z upper-cased. */
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

/* keys_any -- 1 when a key is waiting in the queue, else 0. Does not wait. */
i16 keys_any(void)
{
    timer_sync();
    return host_key_ready() ? 1 : 0;
}

/* keys_get -- waits for a key and takes it from the queue: its ASCII code, or 100h + its scan
 * code for a key without one. The clock keeps running while it waits. */
i16 keys_get(void)
{
    u16 k;
    while (!host_key_ready()) {
        timer_sync();
        host_sleep(1);
    }
    k = host_key_get();
    return (k & 0xff) ? (i16)(k & 0xff) : (i16)(0x100 | (k >> 8));
}
