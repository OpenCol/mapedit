/* timer.h -- the functions, data and records of timer.c, timer_1.c. */
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

extern i16 TIMER_LOW_DEFERRED;
extern i16 TIMER_LOW_PRIORITY;
extern i16 TIMER_LOW_SEMAPHORE;
extern i16 TIMER_LOW_STACKING;
extern i16 TIMER_NOISE_ON;
extern i16 TIMER_SOUND_ON;
extern i32 timer_600_low;
extern i32 timer_60_low;
extern i32 *timer_address;
extern i16 timer_rate;
extern i16 timer_service_active;

#endif
