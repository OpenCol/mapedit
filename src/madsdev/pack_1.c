/* From mapedit-decomp src/madsdev/pack_1 (f55aff8), now maintained here: docs/origin.md. */
#include "msc.h"
#pragma pack(2)   /* MSC 6's default structure packing (/Zp2) */
#include "loader.h"
#include "pack.h"
/* MODULE pack_1: MAPEDIT.EXE segment 1359 (PACK_1.C), file 0x14b96-0x14c0a, 1 functions in source order.
 * cl /c /AM /G2 /Gs /Ot /Ow
 *
 * fn pack_read_memory 0x14b96
 *
 * The game, VICEROY.EXE, links the same module (dos-decomp src/root_1066).
 *
 * THE MEMORY READER FOR IMPLODE/EXPLODE (root code past the C runtime, frame 1066; pascal).
 * pack_read_memory(buf, n) copies up to *n bytes from pack_read_memory_ptr into buf, capped by pack_read_size
 * unless its high word is -1 (unlimited); a zero allowance returns 0. It advances pack_read_memory_ptr
 * and normalises it with mem_normalize, and counts the bytes into pack_read_count. The body is an
 * `_asm` block (the `push ax; or ax,dx; pop ax` zero test and `rep movsb` are not MSC's).
 */


/* pack_read_memory -- 1359:0006, 116 bytes */
/* PORT: pack_read_memory is in port/ (inline assembly) */


/* The module's uninitialised data (communal, BSS). */
char *pack_read_memory_ptr;           /* DS:0x6348 */

