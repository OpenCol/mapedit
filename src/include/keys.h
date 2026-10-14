/* keys.h -- the keyboard: the BIOS key queue, and the clock tick's keyboard work.
 *
 * Defined in sys/keys.c. keys_any says whether a key is waiting and keys_get takes one: its
 * ASCII code, or 100h + the scan code for a key without one (arrows, function keys).
 * keys_fix_alt maps an Alt+letter code to its letter. keys_tick runs on every clock tick
 * (timer.h): it moves the cursor and presses the mouse buttons for keypad keys held down
 * ('-' sets KEYS_SPECIAL_BUTTON), though nothing hooks the keypad, so none ever is; then it
 * steps a screen shake. keys_remove unhooks the keyboard and releases any forced button.
 */
#ifndef MAPEDIT_KEYS_H
#define MAPEDIT_KEYS_H

#include "msc.h"

i16 keys_any(void);
i16 keys_get(void);
void keys_remove(void);
void keys_tick(void);
i16 keys_fix_alt(i16 key);

extern i16 KEYS_SPECIAL_BUTTON;

#endif
