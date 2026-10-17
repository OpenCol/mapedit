/* checks.h -- the checks before saving (port: options.h, save_checks).
 *
 * Defined in editor/checks.c. save_checks_pass numbers the continents (Find Continents) and
 * checks the map for what the game would change or mind. Returns 1 to go on and save: the
 * option is off, nothing was found, or the player chose to save anyway; 0 not to save.
 */
#ifndef MAPEDIT_CHECKS_H
#define MAPEDIT_CHECKS_H

#include "msc.h"

i16 save_checks_pass(void);

#endif
