/* sound.c -- sound_1.ASM and sound_2.ASM. The map editor loads no sound driver: every entry
 * goes through the driver's jump table to the dummy driver, which does nothing. */
#include "msc.h"

i16 sound_initialized;

void sound_driver_null(void) { }
i16 sound_manager(void) { return 0; }
void sound_queue(i16 sound) { (void)sound; }
void SOUND_QUEUE_HOLD(void) { }
void SOUND_QUEUE_FLUSH(void) { }
void sound_system_shutdown(void) { }
void SOUND_MAKE_NOISE(void) { }
i16 SOUND_SYSTEM_SETUP(void) { return 0; }
i16 SOUND_DRIVER_LOAD(void) { return 0; }
i16 SOUND_DRIVER_INIT(void) { return 0; }
void SOUND_DRIVER_REMOVE(void) { }
