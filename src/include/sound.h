/* sound.h -- the functions, data and records of sound.c. */
#ifndef MAPEDIT_SOUND_H
#define MAPEDIT_SOUND_H

#include "msc.h"

i16 SOUND_DRIVER_INIT(void);
i16 SOUND_DRIVER_LOAD(void);
void SOUND_DRIVER_REMOVE(void);
void SOUND_MAKE_NOISE(void);
void SOUND_QUEUE_FLUSH(void);
void SOUND_QUEUE_HOLD(void);
i16 SOUND_SYSTEM_SETUP(void);
void sound_driver_null(void);
i16 sound_initialized(void);
i16 sound_manager(void);
void sound_queue(i16 sound);
void sound_system_shutdown(void);

#endif
