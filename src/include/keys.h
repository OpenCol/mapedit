/* keys.h -- the keyboard: the BIOS key queue.
 *
 * Defined in sys/keys.c. keys_any says whether a key is waiting and keys_get takes one: its
 * ASCII code, or 100h + the scan code for a key without one (arrows, function keys).
 * keys_fix_alt maps an Alt+letter code to its letter.
 */
#ifndef MAPEDIT_KEYS_H
#define MAPEDIT_KEYS_H

#include "msc.h"

i16 keys_any(void);
i16 keys_get(void);
i16 keys_fix_alt(i16 key);

#endif
