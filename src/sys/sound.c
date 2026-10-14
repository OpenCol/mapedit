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

void sound_queue(i16 sound) { (void)sound; }
void sound_system_shutdown(void) { }
