/* keys.c -- the keyboard: the BIOS queue, and the clock tick's keyboard work.
 *
 * keys_any and keys_get read the keyboard queue, which in 1994 was the BIOS's (INT 16h). A key
 * comes back as one number: its ASCII code, or 100h + its scan code for a key without one
 * (arrows, function keys, Alt+letter). keys_fix_alt turns Alt+letter into the letter.
 *
 * keys_tick runs on every clock tick (1994's INT 1Ch handler). It moves the cursor by the
 * keypad keys recorded in `held` -- 4 pixels, 8 with a shift key, 1 with scroll lock on -- but
 * nothing in the program sets `held`, so in practice keys_tick only steps the screen shake
 * (which the editor never starts either).
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

i16 KEYS_SPECIAL_BUTTON;        /* 1 while the keypad's '-' (the special button) is held */

static i8 installed;  /* -1: the keyboard hook is set; nothing sets it */
/* The keypad keys held down: 01h up, 02h down, 04h left, 08h right, 10h the left button,
 * 20h the right, 40h the special button; 80h means something changed since the last tick.
 * Nothing sets it, so it stays 0. */
static u8 held;

/* keys_tick -- the clock tick's keyboard work (1994's INT 1Ch handler; timer.c calls it). When
 * the cursor is the library's own and a keypad key is held or has changed, it sets the forced
 * mouse buttons and moves the cursor, clamped to the 320x200 screen. Then it steps a screen
 * shake, if one is under way. */
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

/* keys_remove -- releases any forced button, and unhooks the keyboard when `installed` is set
 * (which it never is). */
void keys_remove(void)
{
    if (installed)
        host_set_key_hook(0);
    installed = 0;
    MOUSE_FORCE_BUTTON = 0;
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
