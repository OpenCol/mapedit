/* bios.c -- the BIOS data area, where the original reads the keyboard's shift flags and
 * the clock's tick count.
 *
 * The original reads these through fixed pointers into segment 0040h; the port reads the same
 * offsets of msc_bda, a 256-byte array: 10h the equipment flags, 17h the shift flags, 6Ch the
 * 32-bit tick count. Nothing updates them by itself: bios_sync refreshes them from the host,
 * and the clock (timer_sync) calls it whenever the port polls the keyboard, mouse or timer.
 */
#include "msc.h"
#include "machine.h"
#include "records.h"
#include "host.h"

u8 msc_bda[256];   /* the BIOS data area, segment 0040h */

/* bios_sync -- copy the host's elapsed time and shift keys into the BIOS data area. The tick
 * count is derived from the time since start, not counted, so it never drifts; the shift
 * flags' upper four bits (the lock keys, insert) are kept as they are.
 */
void bios_sync(void)
{
    /* the clock: 1193182 / 65536 = 18.2065 ticks a second since start */
    u32 ticks = (u32)((uint64_t)host_ms() * 1193182u / 65536000u);
    msc_bda[0x6c] = (u8)ticks;
    msc_bda[0x6d] = (u8)(ticks >> 8);
    msc_bda[0x6e] = (u8)(ticks >> 16);
    msc_bda[0x6f] = (u8)(ticks >> 24);
    /* the shift flags: right shift 01h, left shift 02h, ctrl 04h, alt 08h */
    msc_bda[0x17] = (u8)((msc_bda[0x17] & 0xf0) | host_shift_flags());
}
