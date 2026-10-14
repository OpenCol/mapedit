/* timer.h -- the clock: the 600 Hz and 60 Hz counters and what runs on them.
 *
 * Defined in sys/timer.c. timer_install starts the clock at about 608.8 Hz: timer_600_low
 * counts every tick, timer_60_low every tenth, and timer_read then returns the 60 Hz count
 * (before timer_install and after timer_remove, the BIOS's 18.2 Hz tick count). The original
 * ran on the PC's timer interrupt; the port has none, so every read first catches the
 * counters up with the host's clock (timer_sync) and runs what each tick would have run: the
 * keypad mouse (keys_tick) and an optional 60 Hz routine (TIMER_ACTIVATE_LOW_PRIORITY).
 * timer_idle is where the port's UI loops rest.
 */
#ifndef MAPEDIT_TIMER_H
#define MAPEDIT_TIMER_H

#include "msc.h"

void TIMER_ACTIVATE_LOW_PRIORITY(void (*routine)(void));
i16 TIMER_GET_COPY_PROTECT(void);
void TIMER_SET_COPY_PROTECT(i16 on);
void TIMER_SET_RATE(u16 d);
void TIMER_SET_SOUND_FLAG(i16 on);
void timer_install(void);
void timer_remove(void);
void timer_sync(void);
void timer_hack(void);
i32 timer_read(void);
i32 timer_read_60(void);
i32 timer_read_600(void);
i32 timer_read_dos(void);

extern i16 TIMER_LOW_DEFERRED;      /* unused */
extern i16 TIMER_LOW_PRIORITY;      /* a 60 Hz routine is installed */
extern i16 TIMER_LOW_SEMAPHORE;     /* set while it runs */
extern i16 TIMER_LOW_STACKING;      /* unused */
extern i16 TIMER_NOISE_ON;          /* the sound flags: set, never read */
extern i16 TIMER_SOUND_ON;
extern i32 timer_600_low;           /* every tick: 608.8 Hz */
extern i32 timer_60_low;            /* every tenth tick: 60.9 Hz */
extern i32 *timer_address;          /* the count timer_read returns */
extern i16 timer_rate;              /* unused */
extern i16 timer_service_active;     /* unused */

void timer_idle(void);

#endif
