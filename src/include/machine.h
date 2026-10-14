/* machine.h -- the BIOS data area.
 *
 * Defined in sys/bios.c. msc_bda stands for the PC's BIOS data area (segment 0040h), where
 * the code reads the keyboard's shift flags (17h) and the clock's tick count (6Ch).
 * bios_sync brings both up to date from the host; every place the port polls the machine
 * (keys, mouse, timer) calls it.
 */
#ifndef MAPEDIT_MACHINE_H
#define MAPEDIT_MACHINE_H

#include "msc.h"

void bios_sync(void);

extern u8 msc_bda[256];

#endif
