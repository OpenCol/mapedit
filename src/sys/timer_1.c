/* From mapedit-decomp src/madsdev/timer_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "font.h"
#include "loader.h"
#include "machine.h"
#include "records.h"
#include "sound.h"
#include "timer.h"
/* MODULE timer_1: MAPEDIT.EXE segment 0d18 (TIMER_1.C), file 0xe786-0xe7c8, 5 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn timer_read     0x0e786
 * fn timer_read_dos 0x0e792
 * fn timer_read_600 0x0e7a2
 * fn timer_read_60  0x0e7aa
 * fn timer_hack     0x0e7b2
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_0c0c).
 *
 * THE TICK COUNTERS (root code, frame 0c0c, from offset 6). Engine code.
 *   - timer_read_60 returns the timer handler's count at DS:0x92e8 (bumped every fifth int 8 in
 *     module 0a0a; timer_address points at it, so timer_read reads the same count). DS:0x8338,
 *     which timer_read_600 returns, counts every interrupt.
 *   - timer_hack installs the timer (timer_install), runs the sound driver's poll
 *     (sound_manager), resets the driver vectors (sound_driver_null) and removes the
 *     timer (timer_remove).
 *     No code calls either function.
 */


i32 *timer_address = (i32 *)(msc_bda + 0x006c);   /* DS:0x267a: the BIOS tick count until the timer is installed */
/* PORT: the counters move when read: the port has no clock interrupt (port/lib/timer.c) */
/* timer_read -- 0d18:0006, 12 bytes */
i32 timer_read(void)
{
    timer_sync();   /* PORT */
    return *timer_address;
}
/* timer_read_dos -- 0d18:0012, 16 bytes */
i32 timer_read_dos(void)
{
    timer_sync();   /* PORT */
    return *(i32 *)(msc_bda + 0x006c);
}
/* timer_read_600 -- 0d18:0022, 8 bytes */
i32 timer_read_600(void)
{
    timer_sync();   /* PORT */
    return timer_600_low;
}
/* timer_read_60 -- 0d18:002a, 8 bytes */
i32 timer_read_60(void)
{
    timer_sync();   /* PORT */
    return timer_60_low;
}
/* timer_hack -- 0d18:0032, 22 bytes */
void timer_hack(void)
{
    timer_install();
    sound_manager();
    sound_driver_null();
    timer_remove();
}

/* The module's last two words (DS:0x267e, 0x2680), after the last function: no count moves. */
i16 timer_rate = 20;                                    /* DS:0x267e */
i16 timer_service_active = 0;                                     /* DS:0x2680 */

