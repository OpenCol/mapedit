/* From mapedit-decomp src/madsdev/xms_6 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
/* MODULE xms_6: MAPEDIT.EXE segment 128b (xms_6.c), file 0x13eb4-0x13f06, 2 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn xms_get      0x13eb4
 * fn xms_free     0x13ef0
 *
 * No VICEROY twin: reconstructed from the shipped bytes.
 *
 * EXTENDED-MEMORY HANDLES (madsdev.lib).
 *   - xms_get(bytes) would allocate an XMS block of (bytes - 1) / 1024 + 1 kilobytes
 *     (function 09h) and return its handle, -1 without a driver or on failure. The shipped
 *     library has it DISABLED: a `jmp` to the failure return is its first instruction, and
 *     the allocation that follows is unreachable. The jump is inside the `_asm` block (a C
 *     `goto` there makes MSC drop the rest). So the map editor's himem_directory_setup
 *     never gets an XMS directory.
 *   - xms_free(handle) releases a handle (function 0Ah) when there is a driver.
 */

extern i16 xms_exists;                      /* DS:0x3c68 (xms_1.c) */
extern void (*xms_controller)(void);    /* DS:0x3c6c (xms_1.c) */
/* xms_get -- 128b:0004, 60 bytes */
/* PORT: xms_get is in port/ (inline assembly) */
i16 xms_get(i32 bytes);

/* xms_free -- 128b:0040, 22 bytes */
/* PORT: xms_free is in port/ (inline assembly) */
void xms_free(i16 handle);


