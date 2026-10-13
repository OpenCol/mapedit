/* keys.h -- the functions, data and records of keys.c, keys_1.c, keys_3.c. */
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
