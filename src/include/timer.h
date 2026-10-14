/* timer.h -- the clock: the 600 Hz and 60 Hz counters and what runs on them.
 *
 * Defined in sys/timer.c. timer_install starts the clock at about 608.8 Hz: timer_600_low
 * counts every tick, timer_60_low every tenth, and timer_read then returns the 60 Hz count
 * (before timer_install and after timer_remove, the BIOS's 18.2 Hz tick count). The original
 * ran on the PC's timer interrupt; the port has none, so every read first catches the
 * counters up with the host's clock (timer_sync). timer_idle is where the port's UI loops
 * rest.
 */
#ifndef MAPEDIT_TIMER_H
#define MAPEDIT_TIMER_H

#include "msc.h"

void TIMER_SET_RATE(u16 d);
void timer_install(void);
void timer_remove(void);
void timer_sync(void);
i32 timer_read(void);

extern i32 timer_600_low;           /* every tick: 608.8 Hz */
extern i32 timer_60_low;            /* every tenth tick: 60.9 Hz */
extern i32 *timer_address;          /* the count timer_read returns */

void timer_idle(void);

#endif
