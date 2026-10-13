/* bios.c -- the BIOS data area (0040:0000), where the source reads the keyboard's shift
 * flags (17h) and the clock's tick count (6Ch) through constant far pointers. The import
 * points those at msc_bda (tools/import.py); bios_sync keeps them current, and every place
 * the port polls the machine (keys, mouse, timer) calls it.
 */
#include "msc.h"
#include "host.h"

u8 msc_bda[256];

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
