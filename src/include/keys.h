/* keys.h -- the keyboard: the BIOS key queue, and the numeric keypad as a mouse.
 *
 * Defined in sys/keys.c. keys_any says whether a key is waiting and keys_get takes one: its
 * ASCII code, or 100h + the scan code for a key without one (arrows, function keys).
 * KEYS_INSTALL turns the numeric keypad into a mouse: while enabled, keypad keys never reach
 * the queue; they move the cursor on every clock tick (keys_tick) and 5, 0 and '.' press the
 * buttons ('-' sets KEYS_SPECIAL_BUTTON). keys_fix_alt maps an Alt+letter code to its letter.
 */
#ifndef MAPEDIT_KEYS_H
#define MAPEDIT_KEYS_H

#include "msc.h"

i16 KEYS_CHECK_INSTALLED(void);
void KEYS_DISABLE(void);
void KEYS_ENABLE(void);
void KEYS_INSTALL(void);
i16 keys_any(void);
i16 keys_get(void);
void keys_remove(void);
void keys_tick(void);
void keys_flush(void);
i16 keys_fix_alt(i16 key);

extern i16 KEYS_SPECIAL_BUTTON;

#endif
