/* timer.c -- the clock: the 600 Hz and 60 Hz counters and what runs on them.
 *
 * In 1994, timer_install reprograms the PIT to divisor 7A8h (1193182 / 1960 = 608.8
 * interrupts a second) and takes INT 8. Its handler counts every interrupt in timer_600_low;
 * every tenth it counts timer_60_low (60.9 a second) and runs the low-priority routine
 * TIMER_LOW_ROUTINE when TIMER_LOW_PRIORITY is set (nothing sets either), and every thirtieth
 * it chains to the BIOS's handler, whose tick (and INT 1Ch: keys.c's keys_tick) then runs 20.3
 * times a second instead of 18.2.
 * The handler's sound steps are not reproduced: the editor plays no sound.
 *
 * The port has no interrupts. timer_sync() brings everything up to the host clock -- the
 * counters, and the periodic work once for each period that has passed -- and runs wherever
 * the editor reads the time or polls its input (the timer_read functions here, the keyboard
 * and the mouse). Between reads nothing moves, which for a program that only ever looks at
 * the clock is the same thing. keys_tick, 1994's INT 1Ch, runs at the chained rate (20.3 Hz)
 * while the timer is installed and on the BIOS tick otherwise.
 *
 * timer_read returns whichever count timer_address points at: the BIOS tick (18.2 Hz) until
 * timer_install, timer_60_low after it.
 */
#include "msc.h"
#include "host.h"
#include "font.h"
#include "keys.h"
#include "loader.h"
#include "machine.h"
#include "pal.h"
#include "records.h"
#include "sound.h"
#include "timer.h"
#include "units.h"
#pragma pack(2)   /* records on 2-byte boundaries, as the 1994 compiler laid them out */

/* The count timer_read returns: the BIOS tick count until the timer is installed, then
 * timer_60_low. */
i32 *timer_address = (i32 *)(msc_bda + 0x006c);

/* The timer_read functions. Port: each calls timer_sync first, since the counters move only
 * when read. */

/* timer_read -- the program's clock: 18.2 Hz before timer_install, 60.9 Hz after. */
i32 timer_read(void)
{
    timer_sync();   /* Port */
    return *timer_address;
}

/* The counters and the handler's state. */
i32 timer_600_low;                      /* every interrupt: 608.8 Hz */
i32 timer_60_low;                       /* every tenth: 60.9 Hz */
i16 TIMER_SOUND_ON, TIMER_NOISE_ON;     /* the sound flags; set, never read */
i16 TIMER_LOW_PRIORITY, TIMER_LOW_SEMAPHORE;
void (*TIMER_LOW_ROUTINE)(void);        /* run every tenth interrupt when TIMER_LOW_PRIORITY; never set */

static u16 divisor;                     /* 0: the BIOS's 65536 */
static int installed;
static uint64_t base_ms, base_count;    /* timer_600_low counted from here */
static i32 last_600;                    /* its value when timer_sync last left it */
static u32 bios_chain;                  /* 1Ch ticks run through the chain */

/* TIMER_SET_RATE -- sets the PIT's channel 0 divisor; 0 restores the BIOS rate. */
void TIMER_SET_RATE(u16 d)
{
    divisor = d;
}

/* rate -- interrupts a second at the current divisor. */
static double rate(void)
{
    return 1193182.0 / (divisor ? divisor : 65536.0);
}

/* interrupt -- the handler's periodic work for interrupt number n (counting from 1). */
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

/* timer_sync -- brings the BIOS data area and the counters up to the host clock, running the
 * periodic work for every interrupt that is due. A counter the program has set itself is
 * counted on from its new value. */
void timer_sync(void)
{
    bios_sync();
    if (installed) {
        uint64_t now = host_ms(), due;
        if (timer_600_low != last_600) {
            /* something other than this loop set the counter: count on from its new
             * value */
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

/* timer_install -- starts the 608.8 Hz clock from 0 and makes timer_read return timer_60_low. */
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

/* timer_remove -- stops the clock and hands timer_read back to the BIOS tick; safe to call
 * when the timer is not installed. */
void timer_remove(void)
{
    if (installed) {
        TIMER_SET_RATE(0);
        timer_address = (i32 *)(msc_bda + 0x6c);
        bios_chain = (u32)msc_bda[0x6c] | (u32)msc_bda[0x6d] << 8 | (u32)msc_bda[0x6e] << 16;
    }
    installed = 0;
}

/* timer_idle -- the port's: a UI loop has nothing to do until the next input or tick. The
 * 1994 loops polled flat out, which a DOS machine could afford; a millisecond's sleep keeps
 * them responsive at a fraction of the CPU. */
void timer_idle(void)
{
    host_idle(1);
}

