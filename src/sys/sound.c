/* sound.c -- the sound entry points, all of which do nothing.
 *
 * The shared code calls the sound system, but the map editor loads no sound driver: in the
 * original every entry reaches the dummy driver, which does nothing. Each function here is that
 * dummy: the ones that return a value return 0 (no sound system, no driver, nothing queued).
 */
#include "msc.h"
#include "records.h"
#include "sound.h"

i16 sound_initialized(void) { return 0; }

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
