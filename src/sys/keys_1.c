/* From mapedit-decomp src/madsdev/keys_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "keys.h"
#include "loader.h"
#include "records.h"
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

