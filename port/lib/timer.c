/* timer.c -- timer_3.ASM and TIMER_2.C: the clock interrupt.
 *
 * timer_install reprograms the PIT to divisor 7A8h (1193182 / 1960 = 608.8 interrupts a
 * second) and takes INT 8. Its handler counts every interrupt in timer_600_low; every second
 * one it plays a sound step; every tenth it counts timer_60_low (60.9 a second), runs the
 * sound manager and the low-priority routine (TIMER_ACTIVATE_LOW_PRIORITY), and every
 * thirtieth it chains to the BIOS's handler, whose tick (and INT 1Ch: keys.c's keypad
 * mouse) then runs 20.3 times a second instead of 18.2.
 *
 * The port has no interrupts. timer_sync() brings everything up to the host clock -- the
 * counters, and the periodic work once for each period that has passed -- and runs wherever
 * the editor reads the time or polls its input (the reads in timer_1.c, through a patch;
 * the keyboard and the mouse here). Between reads nothing moves, which for a program that
 * only ever looks at the clock is the same thing.
 */
#include "msc.h"
#include "keys.h"
#include "machine.h"
#include "records.h"
#include "timer.h"
#include "units.h"
#include "host.h"


i32 timer_600_low;                      /* DS:50b6 */
i32 timer_60_low;                       /* DS:4e72 */
i16 TIMER_SOUND_ON, TIMER_NOISE_ON;
i16 TIMER_LOW_PRIORITY, TIMER_LOW_SEMAPHORE, TIMER_LOW_STACKING, TIMER_LOW_DEFERRED;
void (*TIMER_LOW_ROUTINE)(void);


static u16 divisor;                     /* 0: the BIOS's 65536 */
static int installed;
static uint64_t base_ms, base_count;    /* timer_600_low counted from here */
static i32 last_600;                    /* its value when timer_sync last left it */
static u32 bios_chain;                  /* 1Ch ticks run through the chain */

/* TIMER_SET_RATE -- 105b:0006: the PIT's channel 0 divisor */
void TIMER_SET_RATE(u16 d)
{
    divisor = d;
}

static double rate(void)
{
    return 1193182.0 / (divisor ? divisor : 65536.0);
}

/* the periodic work of the handler, for interrupt number n (1-based) */
static void interrupt(u32 n)
{
    if (n % 10 == 0) {
        timer_60_low++;
        if (TIMER_LOW_PRIORITY && TIMER_LOW_ROUTINE && !TIMER_LOW_SEMAPHORE) {
            TIMER_LOW_SEMAPHORE = 1;
            TIMER_LOW_ROUTINE();
            TIMER_LOW_SEMAPHORE = 0;
        }
    }
    if (n % 30 == 0)
        keys_tick();                    /* the chained BIOS tick, and INT 1Ch */
}

void timer_sync(void)
{
    bios_sync();
    if (installed) {
        uint64_t now = host_ms(), due;
        if (timer_600_low != last_600) {
            /* the program set the counter itself (show_map_gradually zeroes it): count on
               from there */
            base_ms = now;
            base_count = (u32)timer_600_low;
        }
        due = (uint64_t)((double)(now - base_ms) * rate() / 1000.0) + base_count;
        while ((uint64_t)(u32)timer_600_low < due) {
            timer_600_low++;
            interrupt((u32)timer_600_low);
        }
        last_600 = timer_600_low;
    } else {
        /* without the handler, INT 1Ch runs on the BIOS tick */
        u32 t = (u32)msc_bda[0x6c] | (u32)msc_bda[0x6d] << 8 | (u32)msc_bda[0x6e] << 16;
        while (bios_chain < t) {
            bios_chain++;
            keys_tick();
        }
    }
}

/* timer_install -- 0d1c:017d */
void timer_install(void)
{
    TIMER_SOUND_ON = 0;
    TIMER_NOISE_ON = 0;
    timer_60_low = 0;
    timer_600_low = 0;
    TIMER_LOW_SEMAPHORE = 0;
    TIMER_SET_RATE(0x7a8);
    timer_address = &timer_60_low;
    base_ms = host_ms();
    base_count = 0;
    last_600 = 0;
    installed = 1;
}

/* timer_remove -- 0d1c:01f3 */
void timer_remove(void)
{
    if (installed) {
        TIMER_SET_RATE(0);
        timer_address = (i32 *)(msc_bda + 0x6c);
        bios_chain = (u32)msc_bda[0x6c] | (u32)msc_bda[0x6d] << 8 | (u32)msc_bda[0x6e] << 16;
    }
    installed = 0;
}

void TIMER_SET_SOUND_FLAG(i16 on)
{
    TIMER_SOUND_ON = on;
    TIMER_NOISE_ON = 0;
}

void TIMER_ACTIVATE_LOW_PRIORITY(void (*routine)(void))
{
    TIMER_LOW_ROUTINE = routine;
    TIMER_LOW_SEMAPHORE = TIMER_LOW_STACKING = TIMER_LOW_DEFERRED = 0;
    TIMER_LOW_PRIORITY = routine != 0;
}

/* timer_idle -- the port's: a UI loop has nothing to do until the next input or tick. The
   1994 loops polled flat out, which a DOS machine could afford; a millisecond's sleep keeps
   them responsive at a fraction of the CPU. */
void timer_idle(void)
{
    host_idle(1);
}

void TIMER_SET_COPY_PROTECT(i16 on) { (void)on; }
i16 TIMER_GET_COPY_PROTECT(void) { return 0; }
