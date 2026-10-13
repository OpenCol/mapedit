/* machine.h -- the functions, data and records of bios.c. */
#ifndef MAPEDIT_MACHINE_H
#define MAPEDIT_MACHINE_H

#include "msc.h"

void bios_sync(void);

extern u8 msc_bda[256];

#endif
