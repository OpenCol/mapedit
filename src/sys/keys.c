/* keys.c -- the keyboard: the BIOS queue and the keypad as a mouse. */
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
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "video.h"

/* MODULE keys_1: MAPEDIT.EXE segment 0baf (KEYS_1.C), file 0xd0f4-0xd12e, 3 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn keys_any     0x0d0f4
 * fn keys_get     0x0d108
 * fn keys_flush   0x0d11e
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0ae7).
 *
 * THE KEYBOARD (root code, frame 0ae7), through BIOS int 16h.
 *   - keys_any() is non-zero when a key is waiting (function 1, ZF clear).
 *   - keys_get() waits for a key (function 0) and returns its ASCII code, or for an extended
 *     key (ASCII 0) the scan code plus 0x100.
 *   - keys_flush() reads keys until none is waiting.
 * The first two are inline assembly that falls into C labels for the second return.
 */

/* keys_any -- 0baf:0004, 20 bytes */
/* PORT: keys_any is in port/ (inline assembly) */

/* keys_get -- 0baf:0018, 22 bytes */
/* PORT: keys_get is in port/ (inline assembly) */

/* keys_flush -- 0baf:002e, 16 bytes */
void keys_flush(void)
{
    while (keys_any())
        keys_get();
}

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

/* keys.c -- keys_4.ASM (the keyboard interrupt) and keys_1.c's two BIOS calls.
 *
 * keys_any / keys_get are INT 16h: is a key waiting, and take it -- folded the library's way,
 * the ASCII code, or 100h + the scan code for a key without one (arrows, function keys).
 *
 * KEYS_INSTALL takes INT 9 and INT 1Ch to make the numeric keypad a mouse: while the hook is
 * enabled, keypad keys (not the grey ones) never reach the BIOS queue; they set bits in a
 * held-keys byte (7/8/9/4/6/1/2/3 directions, 5 and 0 the left button, '.' the right, '-'
 * a special button), and every clock tick (INT 1Ch, here keys_tick from timer.c) moves the
 * cursor by them: 4 pixels, 8 with shift, 1 with scroll lock on.
 */



i16 KEYS_SPECIAL_BUTTON;

static i8 installed, enabled;
static u8 held;                 /* keys_4's cs:0016: direction and button bits, 80h = changed */

/* scan codes 47h-53h -> bits (keys_4.ASM's table at cs:0018) */
static const u8 keypad_bits[13] = { 0x05, 0x01, 0x09, 0x40, 0x04, 0x10, 0x08, 0x00, 0x06, 0x02, 0x0a, 0x10, 0x20 };

static int hook(int scan, int down, int keypad)
{
    u8 bits;
    if (!enabled || !keypad || scan < 0x47 || scan > 0x53)
        return 0;
    bits = keypad_bits[scan - 0x47];
    if (down)
        held |= (u8)(bits | 0x80);
    else
        held = (u8)((held & (u8)~bits) | 0x80);
    return 1;
}

/* the INT 1Ch handler -- keys_4.ASM 00a4 */
void keys_tick(void)
{
    if (MOUSE_KNOWN_MODE && held) {
        u8 b = held, shift = msc_bda[0x17];
        i16 dx, dy, x, y;
        held &= 0x7f;
        MOUSE_FORCE_BUTTON = (i16)((b >> 4) & 3);
        KEYS_SPECIAL_BUTTON = (i16)((b >> 6) & 1);
        dx = (b & 0x04) ? -1 : (b & 0x08) ? 1 : 0;
        dy = (b & 0x01) ? -1 : (b & 0x02) ? 1 : 0;
        if (!(shift & 0x10)) {          /* scroll lock off: 4, or 8 with a shift key */
            dx *= 4;
            dy *= 4;
            if (shift & 3) {
                dx *= 2;
                dy *= 2;
            }
        }
        x = (i16)(MOUSE_X_TEMP + dx);
        x = x < 0 ? 0 : x > 0x13f ? 0x13f : x;
        y = (i16)(MOUSE_Y_TEMP + dy);
        y = y < 0 ? 0 : y > 0xc7 ? 0xc7 : y;
        mouse_keypad_move(x, y);
    }
    if (MCGA_SHAKES)
        MCGA_SHAKE();
}

void KEYS_INSTALL(void)
{
    held = 0;
    MOUSE_FORCE_BUTTON = 0;
    host_set_key_hook(hook);
    installed = enabled = -1;
}

void keys_remove(void)
{
    if (installed)
        host_set_key_hook(0);
    installed = 0;
    MOUSE_FORCE_BUTTON = 0;
}

i16 KEYS_CHECK_INSTALLED(void) { return installed; }
void KEYS_ENABLE(void) { enabled = -1; }
void KEYS_DISABLE(void) { enabled = 0; held = 0; }

/* keys_any -- 0baf:0004: INT 16h AH=01h */
i16 keys_any(void)
{
    timer_sync();
    return host_key_ready() ? 1 : 0;
}

/* keys_get -- 0baf:0018: INT 16h AH=00h (waits), folded: ASCII, or 100h + scan code */
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
