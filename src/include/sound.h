/* sound.h -- the sound entry points: the editor plays no sound.
 *
 * Defined in sys/sound.c. The shared code calls the sound driver's entry points; the map
 * editor loads no driver, so each does nothing; sound_initialized returns 0.
 */
#ifndef MAPEDIT_SOUND_H
#define MAPEDIT_SOUND_H

#include "msc.h"

i16 sound_initialized(void);
void sound_queue(i16 sound);
void sound_system_shutdown(void);

#endif
